#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>


class RobotController : public rclcpp::Node {
    public:
        RobotController() : Node("control_publisher"){
            joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
            RCLCPP_INFO(this->get_logger(), "RobotController node initialized.");
            timer_ = this->create_wall_timer(std::chrono::milliseconds(5000), std::bind(&RobotController::publishJointStates, this));
        }
    private:

        void publishJointStates(){
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

            message.position = {
                1.0,
                1.57,
                -1.0,
                -1.57,
                0.0,
                0.0
            };

            message.velocity = {
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0
            };

            joint_pub_->publish(message);
            // RCLCPP_INFO(this->get_logger(), "Published joint states.");

        }

        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
        size_t count_ = 0;

};

int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotController>());
    rclcpp::shutdown();
    return 0;
}