#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <iostream>  // Add this for std::cout, std::cin

class PositionPub : public rclcpp::Node {
public:
    PositionPub() : Node("position_publisher"),  // Changed name to avoid conflict
                    target_positions_(6, 0.0)
    {
        position_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("/joint_commands", 10);
        RCLCPP_INFO(this->get_logger(), "Control input node initialized.");
        
        // Start the input process
        publish_target_positions();
    }

private:
    void publish_target_positions(){
        std::cout << "Input 6 joint positions:\n" << std::endl;
        
        // Read each joint position individually
        for (int i = 0; i < 6; i++) {
            std::cout << "Joint " << (i+1) << ": ";
            std::cin >> target_positions_[i];
        }
        
        // Create and publish the message
        auto message = sensor_msgs::msg::JointState();
        message.header.stamp = this->get_clock()->now();
        
        message.name = {
            "shoulder_pan_joint",
            "shoulder_lift_joint", 
            "elbow_joint",
            "wrist_1_joint",
            "wrist_2_joint",
            "wrist_3_joint"
        };
        
        message.position = target_positions_;
        message.velocity = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        
        position_pub_->publish(message);
        
        std::cout << "Published target positions!" << std::endl;
        
        // Ask if user wants to send another command
        char choice;
        std::cout << "Send another command? (y/n): ";
        std::cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            publish_target_positions();  // Recursive call for another input
        } else {
            rclcpp::shutdown();  // Exit the program
        }
    }

    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr position_pub_;
    std::vector<double> target_positions_;
};

int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PositionPub>());
    rclcpp::shutdown();
    return 0;
}