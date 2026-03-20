// ncurses terminal UI client for the limit order book
#include "../include/lob/types.h"
#include "../include/lob/feed_handler.h"
#include <ncurses.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cstring>
#include <chrono>
#include <ctime>
#include <iostream>

// ─── layout constants ────────────────────────────────────────────────────────
static const int BOOK_W      = 36;
static const int ENTRY_W     = 30;
static const int HISTORY_H   = 12;

// ─── color pair ids ──────────────────────────────────────────────────────────
enum Colors {
    C_HEADER  = 1,   // white on dark blue
    C_BID     = 2,   // green on black
    C_ASK     = 3,   // red on black
    C_FILL    = 4,   // yellow on black
    C_NORMAL  = 5,   // white on black
    C_HIGHLIGHT = 6, // black on cyan
};

// ─── state ───────────────────────────────────────────────────────────────────
enum class OrderStatus { ACTIVE, FILLED, CANCELLED, EXPIRED };

static std::string status_str(OrderStatus s) {
    switch (s) {
        case OrderStatus::ACTIVE:    return "ACTIVE";
        case OrderStatus::FILLED:    return "FILLED";
        case OrderStatus::CANCELLED: return "CANCELLED";
        case OrderStatus::EXPIRED:   return "EXPIRED";
    }
    return "UNKNOWN";
}

static std::string now_str() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));
    return std::string(buf);
}

struct TradeRecord {
    uint32_t    order_id;
    std::string ticker;
    int         qty;
    int32_t     price;
    uint8_t     side;        // 0=BUY 1=SELL
    OrderStatus status;
    std::string created_at;  // timestamp string when first added
    std::string updated_at;  // timestamp string of last edit
};

struct UIState {
    // windows
    WINDOW* win_book;
    WINDOW* win_entry;
    WINDOW* win_history;
    WINDOW* win_status;

    // local order cache: order_id -> sent message
    std::unordered_map<uint32_t, OrderMessage> sent_orders;

    // trade history
    std::vector<TradeRecord> history;

    // server socket
    int sock = -1;

    // status message
    std::string status_msg;
};

// ─── helpers ─────────────────────────────────────────────────────────────────
static std::string price_str(int32_t ticks) {
    // convert integer ticks back to dollars: 10005 -> "$100.05"
    std::ostringstream ss;
    ss << "$" << (ticks / 100) << "." 
       << (ticks % 100 < 10 ? "0" : "") << (ticks % 100);
    return ss.str();
}

static void draw_border_title(WINDOW* w, const char* title, int color_pair) {
    wattron(w, COLOR_PAIR(color_pair) | A_BOLD);
    box(w, 0, 0);
    int width = getmaxx(w);
    int tx = (width - strlen(title) - 2) / 2;
    mvwprintw(w, 0, tx, " %s ", title);
    wattroff(w, COLOR_PAIR(color_pair) | A_BOLD);
}

static void set_status(UIState& ui, const std::string& msg) {
    ui.status_msg = msg;
    wclear(ui.win_status);
    wattron(ui.win_status, COLOR_PAIR(C_FILL));
    mvwprintw(ui.win_status, 0, 1, "%s", msg.c_str());
    wattroff(ui.win_status, COLOR_PAIR(C_FILL));
    wrefresh(ui.win_status);
}

// ─── window drawing ──────────────────────────────────────────────────────────
static void draw_book(UIState& ui,
                      const std::vector<std::pair<int32_t,int>>& asks,
                      const std::vector<std::pair<int32_t,int>>& bids)
{
    wclear(ui.win_book);
    draw_border_title(ui.win_book, "ORDER BOOK", C_HEADER);

    int row = 1;
    // asks (show up to 5, lowest ask at bottom so reverse)
    mvwprintw(ui.win_book, row++, 2, "%-18s %10s", "ASKS", "QTY");
    wattron(ui.win_book, COLOR_PAIR(C_ASK));
    int ask_start = std::max(0, (int)asks.size() - 5);
    for (int i = (int)asks.size()-1; i >= ask_start; --i) {
        mvwprintw(ui.win_book, row++, 2, "  %-16s %8d",
                  price_str(asks[i].first).c_str(), asks[i].second);
    }
    wattroff(ui.win_book, COLOR_PAIR(C_ASK));

    // spread line
    wattron(ui.win_book, COLOR_PAIR(C_HIGHLIGHT) | A_BOLD);
    if (!bids.empty() && !asks.empty()) {
        int32_t spread = asks.front().first - bids.front().first;
        mvwprintw(ui.win_book, row++, 2, "  spread: %-24s",
                  price_str(spread).c_str());
    } else {
        mvwprintw(ui.win_book, row++, 2, "%-32s", "  -- no spread --");
    }
    wattroff(ui.win_book, COLOR_PAIR(C_HIGHLIGHT) | A_BOLD);

    // bids (show up to 5)
    mvwprintw(ui.win_book, row++, 2, "%-18s %10s", "BIDS", "QTY");
    wattron(ui.win_book, COLOR_PAIR(C_BID));
    int bid_end = std::min((int)bids.size(), 5);
    for (int i = 0; i < bid_end; ++i) {
        mvwprintw(ui.win_book, row++, 2, "  %-16s %8d",
                  price_str(bids[i].first).c_str(), bids[i].second);
    }
    wattroff(ui.win_book, COLOR_PAIR(C_BID));

    wrefresh(ui.win_book);
}

static void draw_history(UIState& ui) {
    wclear(ui.win_history);
    draw_border_title(ui.win_history, "TRADE HISTORY", C_HEADER);

    // header row
    wattron(ui.win_history, A_BOLD | COLOR_PAIR(C_NORMAL));
    mvwprintw(ui.win_history, 1, 2, "%-4s %-6s %-5s %-10s %-4s %-9s %-8s %-8s",
              "ID", "TICKER", "SIDE", "PRICE", "QTY", "STATUS", "CREATED", "UPDATED");
    wattroff(ui.win_history, A_BOLD | COLOR_PAIR(C_NORMAL));

    int row = 2;
    int start = std::max(0, (int)ui.history.size() - (HISTORY_H - 3));
    for (int i = start; i < (int)ui.history.size(); ++i) {
        auto& t = ui.history[i];

        // color by status
        int cpair = C_NORMAL;
        int attrs  = 0;
        switch (t.status) {
            case OrderStatus::ACTIVE:    cpair = C_BID;    attrs = A_BOLD;   break;
            case OrderStatus::FILLED:    cpair = C_FILL;   attrs = A_DIM;    break;
            case OrderStatus::CANCELLED: cpair = C_ASK;    attrs = A_DIM;    break;
            case OrderStatus::EXPIRED:   cpair = C_NORMAL; attrs = A_DIM;    break;
        }

        wattron(ui.win_history, COLOR_PAIR(cpair) | attrs);
        mvwprintw(ui.win_history, row++, 2,
                  "%-4u %-6s %-5s %-10s %-4d %-9s %-8s %-8s",
                  t.order_id,
                  t.ticker.c_str(),
                  t.side == 0 ? "BUY" : "SELL",
                  price_str(t.price).c_str(),
                  t.qty,
                  status_str(t.status).c_str(),
                  t.created_at.c_str(),
                  t.updated_at.c_str());
        wattroff(ui.win_history, COLOR_PAIR(cpair) | attrs);
    }
    wrefresh(ui.win_history);
}

// ─── entry panel helpers ─────────────────────────────────────────────────────
// Print prompt on `row`, read input on `row+1`. Returns row after input.
static std::string entry_prompt_str(WINDOW* w, int& row, const char* prompt) {
    wattron(w, COLOR_PAIR(C_NORMAL));
    mvwprintw(w, row++, 2, "%s", prompt);
    wattroff(w, COLOR_PAIR(C_NORMAL));
    wattron(w, COLOR_PAIR(C_HIGHLIGHT));
    mvwprintw(w, row, 2, "%-24s", "");   // clear input line
    wmove(w, row, 2);
    wrefresh(w);
    echo();
    curs_set(1);
    char buf[64] = {};
    wgetnstr(w, buf, 32);
    noecho();
    wattroff(w, COLOR_PAIR(C_HIGHLIGHT));
    row++;  // advance past input line
    return std::string(buf);
}

static int32_t entry_prompt_int(WINDOW* w, int& row, const char* prompt) {
    std::string s = entry_prompt_str(w, row, prompt);
    try { return std::stoi(s); } catch (...) { return 0; }
}

// ─── order entry flows ───────────────────────────────────────────────────────
static void flow_add(UIState& ui) {
    WINDOW* w = ui.win_entry;
    wclear(w);
    draw_border_title(w, "ADD ORDER", C_HEADER);
    wrefresh(w);

    int row = 2;
    uint8_t side    = (uint8_t)entry_prompt_int(w, row, "Side (0=BUY, 1=SELL):");
    int32_t price   = entry_prompt_int(w, row, "Price (ticks, e.g. 10005):");
    uint32_t qty    = (uint32_t)entry_prompt_int(w, row, "Quantity:");
    std::string ticker = entry_prompt_str(w, row, "Ticker (e.g. AAPL):");

    // build and send message
    OrderMessage msg{};
    msg.msg_type      = 'A';
    msg.side          = side;
    msg.price         = price;
    msg.quantity      = qty;
    msg.order_type    = 1;   // LIMIT
    msg.time_in_force = 1;   // GTC
    msg.fill_policy   = 0;   // NORMAL
    msg.stop_value    = 0;
    strncpy(msg.ticker, ticker.c_str(), 8);

    write(ui.sock, &msg, sizeof(msg));

    ExecutionReport report{};
    read(ui.sock, &report, sizeof(report));

    msg.order_id = report.order_id;
    ui.sent_orders[report.order_id] = msg;

    std::string ts = now_str();
    // add to history or update existing
    bool found = false;
    for (auto& t : ui.history) {
        if (t.order_id == report.order_id) {
            t.status     = OrderStatus::ACTIVE;
            t.updated_at = ts;
            found = true; break;
        }
    }
    if (!found) {
        ui.history.push_back({
            report.order_id,
            ticker,
            (int)qty,
            price,
            side,
            OrderStatus::ACTIVE,
            ts,   // created_at
            ts    // updated_at
        });
    }
    if (report.status == 'F') {
        std::string action = report.side == 0 ? "Bought" : "Sold";
        set_status(ui, "Trade executed. " + action  + " " + std::to_string(report.fill_qty) + " shares of " 
                + msg.ticker + " at $" + std::to_string(report.fill_price) + ".");
    } else {
        set_status(ui, "Order added. ID=" + std::to_string(report.order_id)
                + "  status=" + (char)report.status);
    }
}

static void flow_cancel(UIState& ui) {
    WINDOW* w = ui.win_entry;
    wclear(w);
    draw_border_title(w, "CANCEL ORDER", C_HEADER);

    int row = 2;
    wattron(w, COLOR_PAIR(C_NORMAL) | A_BOLD);
    mvwprintw(w, row++, 2, "Known orders:");
    wattroff(w, COLOR_PAIR(C_NORMAL) | A_BOLD);
    for (auto& [id, om] : ui.sent_orders) {
        mvwprintw(w, row++, 2, "  ID=%-4u %-5s %s x%u",
                  id,
                  om.side == 0 ? "BUY" : "SELL",
                  price_str(om.price).c_str(),
                  om.quantity);
        if (row >= getmaxy(w) - 4) break;
    }
    row++;  // blank line before prompt

    uint32_t id = (uint32_t)entry_prompt_int(w, row, "Order ID to cancel:");

    OrderMessage msg{};
    msg.msg_type = 'C';
    msg.order_id = id;
    write(ui.sock, &msg, sizeof(msg));

    ExecutionReport report{};
    read(ui.sock, &report, sizeof(report));

    ui.sent_orders.erase(id);

    // update history status
    std::string ts = now_str();
    for (auto& t : ui.history) {
        if (t.order_id == id) {
            t.status     = OrderStatus::CANCELLED;
            t.updated_at = ts;
            break;
        }
    }

    set_status(ui, "Cancelled ID=" + std::to_string(id));
}

static void flow_modify(UIState& ui) {
    WINDOW* w = ui.win_entry;
    wclear(w);
    draw_border_title(w, "MODIFY ORDER", C_HEADER);

    int row = 2;
    wattron(w, COLOR_PAIR(C_NORMAL) | A_BOLD);
    mvwprintw(w, row++, 2, "Known orders:");
    wattroff(w, COLOR_PAIR(C_NORMAL) | A_BOLD);
    for (auto& [id, om] : ui.sent_orders) {
        mvwprintw(w, row++, 2, "  ID=%-4u %-5s %s x%u",
                  id,
                  om.side == 0 ? "BUY" : "SELL",
                  price_str(om.price).c_str(),
                  om.quantity);
        if (row >= getmaxy(w) - 8) break;
    }
    row++;

    uint32_t id = (uint32_t)entry_prompt_int(w, row, "Order ID to modify:");

    auto it = ui.sent_orders.find(id);
    if (it == ui.sent_orders.end()) {
        set_status(ui, "Order ID not found locally");
        return;
    }
    auto& old = it->second;

    // show current values, take new ones
    wclear(w);
    draw_border_title(w, "MODIFY ORDER", C_HEADER);
    row = 2;

    wattron(w, COLOR_PAIR(C_HIGHLIGHT));
    mvwprintw(w, row++, 2, "Current price : %s", price_str(old.price).c_str());
    mvwprintw(w, row++, 2, "Current qty   : %u", old.quantity);
    wattroff(w, COLOR_PAIR(C_HIGHLIGHT));
    row++;

    int32_t new_price = entry_prompt_int(w, row, "New price (0=keep):");
    if (new_price == 0) new_price = old.price;

    int32_t new_qty = entry_prompt_int(w, row, "New qty (0=keep):");
    if (new_qty == 0) new_qty = (int32_t)old.quantity;

    OrderMessage msg = old;
    msg.msg_type = 'M';
    msg.order_id = id;
    msg.price    = new_price;
    msg.quantity = (uint32_t)new_qty;

    write(ui.sock, &msg, sizeof(msg));

    ExecutionReport report{};
    read(ui.sock, &report, sizeof(report));

    ui.sent_orders[id] = msg;

    // update history
    std::string ts = now_str();
    for (auto& t : ui.history) {
        if (t.order_id == id) {
            t.price      = new_price;
            t.qty        = new_qty;
            t.updated_at = ts;
            break;
        }
    }

    if (report.status == 'F') {
        std::string action = report.side == 0 ? "Bought" : "Sold";
        set_status(ui, "Trade executed. " + action  + " " + std::to_string(report.fill_qty) + " shares of " 
                + msg.ticker + " at $" + std::to_string(report.fill_price) + ".");
    } else {
        set_status(ui, "Modified ID=" + std::to_string(id));
    }
}

// ─── main ─────────────────────────────────────────────────────────────────────
int main() {
    // ── connect to server ──
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9001);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Connection to server failed. Is lob_server running?\n";
        return 1;
    }

    // ── ncurses init ──
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);

    if (!has_colors()) {
        endwin();
        std::cerr << "Terminal does not support colors\n";
        return 1;
    }
    start_color();
    init_pair(C_HEADER,    COLOR_WHITE,  COLOR_BLUE);
    init_pair(C_BID,       COLOR_GREEN,  COLOR_BLACK);
    init_pair(C_ASK,       COLOR_RED,    COLOR_BLACK);
    init_pair(C_FILL,      COLOR_YELLOW, COLOR_BLACK);
    init_pair(C_NORMAL,    COLOR_WHITE,  COLOR_BLACK);
    init_pair(C_HIGHLIGHT, COLOR_BLACK,  COLOR_CYAN);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // ── create windows ──
    // [ORDER BOOK | ENTRY PANEL]
    // [TRADE HISTORY            ]
    // [STATUS BAR               ]
    int book_h    = rows - HISTORY_H - 2;
    WINDOW* win_book    = newwin(book_h,    BOOK_W,  0, 0);
    WINDOW* win_entry   = newwin(book_h,    ENTRY_W, 0, BOOK_W + 1);
    WINDOW* win_history = newwin(HISTORY_H, cols,    book_h, 0);
    WINDOW* win_status  = newwin(1,         cols,    rows-1, 0);

    UIState ui;
    ui.win_book    = win_book;
    ui.win_entry   = win_entry;
    ui.win_history = win_history;
    ui.win_status  = win_status;
    ui.sock        = sock;

    // ── initial draw ──
    // empty book to start
    draw_book(ui, {}, {});
    draw_history(ui);

    // draw command bar on entry panel
    wclear(win_entry);
    draw_border_title(win_entry, "COMMANDS", C_HEADER);
    wattron(win_entry, COLOR_PAIR(C_NORMAL));
    mvwprintw(win_entry, 2, 2, "[A] Add order");
    mvwprintw(win_entry, 3, 2, "[C] Cancel order");
    mvwprintw(win_entry, 4, 2, "[M] Modify order");
    mvwprintw(win_entry, 5, 2, "[Q] Quit");
    wattroff(win_entry, COLOR_PAIR(C_NORMAL));
    wrefresh(win_entry);

    set_status(ui, "Connected to server. Press A/C/M/Q.");

    // ── main event loop ──
    while (true) {
        int ch = getch();
        switch (ch) {
            case 'a': case 'A':
                flow_add(ui);
                break;
            case 'c': case 'C':
                flow_cancel(ui);
                break;
            case 'm': case 'M':
                flow_modify(ui);
                break;
            case 'q': case 'Q':
                goto done;
        }

        // redraw after each action
        draw_history(ui);

        // reset entry panel to command menu
        wclear(win_entry);
        draw_border_title(win_entry, "COMMANDS", C_HEADER);
        wattron(win_entry, COLOR_PAIR(C_NORMAL));
        mvwprintw(win_entry, 2, 2, "[A] Add order");
        mvwprintw(win_entry, 3, 2, "[C] Cancel order");
        mvwprintw(win_entry, 4, 2, "[M] Modify order");
        mvwprintw(win_entry, 5, 2, "[Q] Quit");
        wattroff(win_entry, COLOR_PAIR(C_NORMAL));
        wrefresh(win_entry);
    }

done:
    endwin();
    close(sock);
    return 0;
}