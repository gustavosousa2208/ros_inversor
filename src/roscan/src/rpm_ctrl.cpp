#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>  // Include this for ioctl
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using std::placeholders::_1;

int waitForMessage(int sock);
int createSocket();

class MinimalSubscriber : public rclcpp::Node
{
public:
  MinimalSubscriber()
  : Node("minimal_subscriber")
  {
    subscription_ = this->create_subscription<std_msgs::msg::String>(
      "topic", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));
  }

private:
  void topic_callback(const std_msgs::msg::String::SharedPtr msg) const
  {
    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
  }
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char ** argv) {
  (void) argc;
  (void) argv;


  printf("node started\n");


  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();

  return 0;
}

int createSocket(){
  int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (sock == -1) {
      std::cerr << "Error opening socket" << std::endl;
      return 1;
  } else {
      std::cout << "Socket opened successfully." << std::endl;
  }

  struct ifreq ifr;
  std::strcpy(ifr.ifr_name, "can0");  // Replace "can0" with your desired interface name
  ioctl(sock, SIOCGIFINDEX, &ifr);

  struct sockaddr_can addr;
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  int bret;
  bret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));

  if (bret == -1) {
      std::cerr << "Error binding socket" << std::endl;
      return 1;
  } else {
      std::cout << "Socket binded successfully." << std::endl;
  }

  return sock;
}

int waitForMessage(int sock){
  struct can_frame frame;
  while (true) {
    ssize_t nbytes = read(sock, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        std::cerr << "Error reading from socket" << std::endl;
        break;
    }

    // de 8 a 15 e do frame 1
    if (frame.can_id == 0x680){
      std::cout << "Dados do inversor " << std::hex << static_cast<int>(frame.data[0]) << " " << std::hex << static_cast<int>(frame.data[1]) << " " << std::endl;
      if(((frame.data[1] >> 4) & 1) == 1) {
          std::cout << "REMOTO" << std::endl;
      }
      if(((frame.data[1] >> 7) & 1) == 1) {
          std::cout << "FALHA" << std::endl;
      }
      if(((frame.data[1] >> 5) & 1) == 1) {
          std::cout << "SUBTENSAO" << std::endl;
      }
    }
  }

  return 0;
}