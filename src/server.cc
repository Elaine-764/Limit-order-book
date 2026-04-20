// TCP server implementation
#include "../include/lob/exchange.h"
#include "../include/lob/feed_handler.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// TODO: when both orders are filled, error involving order_id not found
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
    // std::cout << "Exchange created successfully\n";
    
    // 6. message loop
    while (true) {
        // std::cout << "Running loop...\n";
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

            std::cout << "report USER ID should be: "  << new_id << "\n";

            if (fills.empty()) {
                std::cout << "Filled order empty - no trades happened" << "\n";
                uint32_t count = 1;
                write(client_fd, &count, sizeof(count)); 

                ExecutionReport report = buildReport(
                    static_cast<uint32_t>(new_id),
                    order.side,
                    'A',
                    0,
                    0,
                    msg.quantity,
                    msg.ticker
                );
                write(client_fd, &report, sizeof(report));
                std::cout << "Actual USERID: " << report.order_id << "\n";
            } else {
                std::cout << "Filled orders found!" << "\n";
                uint32_t count = static_cast<uint32_t>(fills.size());
                write(client_fd, &count, sizeof(count));

                for (auto& f : fills) {
                    std::cout << "fetching report status" << "\n";
                    char status = exchange.CheckIfOrderExists(f.order_id) ? 'P' : 'F';
                    std::cout << "Actual USERID: " << f.order_id << ", Report Status: " << status << "\n";
                    uint32_t remaining_qty = exchange.CheckIfOrderExists(f.order_id) ? exchange.getRemainingQty(f.order_id) : 0;
                    ExecutionReport report = buildReport(
                        static_cast<uint32_t>(f.order_id),
                        static_cast<uint8_t>(f.side),
                        status,
                        f.price,
                        static_cast<uint32_t>(f.qty),
                        remaining_qty,
                        msg.ticker
                    );
                    write(client_fd, &report, sizeof(report));
                }
            }
        } else if (msg.msg_type == 'C') {
            try {
                exchange.cancelOrder(msg.order_id);
            } catch (...) {
                throw std::invalid_argument("unknown order");
            }
            
            exchange.print();
            ExecutionReport report = buildReport(msg.order_id, msg.side, 'C', 0, 0, 0, msg.ticker);
            write(client_fd, &report, sizeof(report));
        } else if (msg.msg_type == 'M'){
            Order order = deserialize(msg);
            int new_id = order.order_id;
            std::vector<FillResult> fills = exchange.addOrder(order);
            exchange.print();

            if (fills.empty()) {
                ExecutionReport report = buildReport(
                    static_cast<uint32_t>(new_id),
                    order.side,
                    'A',
                    0,
                    0,
                    msg.quantity,
                    msg.ticker
                );
                write(client_fd, &report, sizeof(report));
            } else {
                for (auto& f : fills) {
                    char status = exchange.CheckIfOrderExists(f.order_id) ? 'P' : 'F';
                    uint32_t remaining_qty = exchange.CheckIfOrderExists(f.order_id) ? exchange.getRemainingQty(f.order_id) : 0;
                    ExecutionReport report = buildReport(
                        static_cast<uint32_t>(f.order_id),
                        static_cast<uint8_t>(f.side),
                        status,
                        f.price,
                        static_cast<uint32_t>(f.qty),
                        remaining_qty,
                        msg.ticker
                    );
                    write(client_fd, &report, sizeof(report));
                }
            }
        }
    }
    close(client_fd);
    close(server_fd);
    return 0;
}