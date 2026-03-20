// TCP server implementation
#include "../include/lob/exchange.h"
#include "../include/lob/feed_handler.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// TODO: Implement TCP server with:
// - Socket setup
// - Client connections
// - Message deserialization from OrderMessage
// - Order routing to OrderBook
// - Trade execution reporting

int main() {
    std::cout << "Order Book Server starting...\n";
    // 1. create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. bind to port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9001);
    bind(server_fd, (sockaddr*)&addr, sizeof(addr));

    // 3. listen
    listen(server_fd, 5);
    std::cout << "Server listening on port 9001\n";

    // 4. accept one client  
    int client_fd = accept(server_fd, nullptr, nullptr);
    std::cout << "Client connected\n";
     
    // 5. create exchange
    Exchange exchange;
    
    // 6. message loop
    while (true) {
        OrderMessage msg;
        ssize_t bytes = read(client_fd, &msg, sizeof(msg));
        
        if (bytes == 0) { 
            std::cout << "Client disconnected\n"; 
            break; 
        }
        if (bytes != sizeof(msg)) break;
        if (msg.msg_type == 'A') {
            Order order = deserialize(msg);
            int new_id = order.order_id;

            // std::cout << "Successfully Deserialized" << "\n";
            std::string ticker = order.ticker;
            std::vector<FillResult> fills = exchange.addOrder(order);
            exchange.print();

            if (fills.empty()) {
                ExecutionReport report{};
                report.order_id      = new_id;
                order.side           = order.side;
                report.status        = 'A';   // acknowledged, resting
                report.fill_qty      = 0;
                report.remaining_qty = msg.quantity;
                report.fill_price    = 0;
                write(client_fd, &report, sizeof(report));
            } else {
                for (auto& f : fills) {
                    ExecutionReport report{};
                    report.order_id      = new_id;
                    order.side           = order.side;
                    report.status        = exchange.CheckIfOrderExists(new_id) ? 'P' : 'F';
                    report.fill_qty      = f.qty;
                    report.fill_price    = f.price;
                    report.remaining_qty = exchange.getRemainingQty(new_id);
                    write(client_fd, &report, sizeof(report));
                }
            }
            // std::cout << "Successfully added to order book" << "\n";
            // std::cout << "Price " << order.price << "\n";
            // std::cout << "Quantity" << order.quantity << "\n";
            // bool filled = false;
            // int new_quantity = order.quantity;
            // int filled_qty = 0;
            // if (exchange.CheckIfOrderExists(order.order_id)) {
            //     Order &new_order = exchange.findOrder(order.order_id);
            //     new_quantity = new_order.quantity;
            //     if (new_quantity != initial_quantity) {
            //         filled = true;
            //         filled_qty = new_quantity - initial_quantity;
            //     }
            // } else {
            //     filled = true;
            //     filled_qty = initial_quantity;
            // }
        
            // std::cout << "creating report" << std::endl;
            // ExecutionReport report{static_cast<uint32_t>(order.order_id), static_cast<uint8_t>(order.side), 'N', 0, 0, 0};
            // // report.order_id      = order.order_id;
            // // order.side           = order.side;
            // report.status        = filled ? 'F' : 'A';
            // report.fill_qty      = filled_qty;
            // report.remaining_qty = new_quantity;
            // report.fill_price    = filled ? tradePrice : 0;
            // std::cout << "writing report to client" << std::endl;
            // write(client_fd, &report, sizeof(report));
        } else if (msg.msg_type == 'C') {
            exchange.cancelOrder(msg.order_id);
            exchange.print();
            ExecutionReport report = buildReport(msg.order_id, msg.side, 'C', 0, 0, 0, msg.ticker);
            write(client_fd, &report, sizeof(report));
        } else if (msg.msg_type == 'M'){
            Order order = deserialize(msg);
            int new_id = order.order_id;
            std::vector<FillResult> fills = exchange.addOrder(order);
            exchange.print();

            if (fills.empty()) {
                ExecutionReport report{};
                report.order_id      = new_id;
                order.side           = order.side;
                report.status        = 'A';   // acknowledged, resting
                report.fill_qty      = 0;
                report.remaining_qty = msg.quantity;
                report.fill_price    = 0;
                write(client_fd, &report, sizeof(report));
            } else {
                for (auto& f : fills) {
                    ExecutionReport report{};
                    report.order_id      = new_id;
                    order.side           = order.side;
                    report.status        = exchange.CheckIfOrderExists(new_id) ? 'P' : 'F';
                    report.fill_qty      = f.qty;
                    report.fill_price    = f.price;
                    report.remaining_qty = exchange.getRemainingQty(new_id);
                    write(client_fd, &report, sizeof(report));
                }
            }
        }
    }
    close(client_fd);
    close(server_fd);
    return 0;
}