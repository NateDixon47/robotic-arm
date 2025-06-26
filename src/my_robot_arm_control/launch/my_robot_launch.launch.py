from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    declared_arguments = []
    
    declared_arguments.append(
        DeclareLaunchArgument(
            "ur_type",
            default_value="ur5e",
            description="Type/series of used UR robot.",
        )
    )

    # Initialize Arguments
    ur_type = LaunchConfiguration("ur_type")

    # Robot description - using the xacro file with proper arguments
    robot_description_content = Command([
        PathJoinSubstitution([FindExecutable(name="xacro")]),
        " ",
        PathJoinSubstitution([FindPackageShare("ur_description"), "urdf", "ur.urdf.xacro"]),
        " ",
        "name:=ur",
        " ",
        "ur_type:=", ur_type,
        " ",
        "tf_prefix:=''",
        " ",
        "safety_limits:=false"
    ])

    robot_description = {
        "robot_description": ParameterValue(value=robot_description_content, value_type=str)
    }

    # Your custom controller (replaces joint_state_publisher_gui)
    robot_controller_node = Node(
        package="my_robot_arm_control",
        executable="robot_controller",
        name="robot_controller",
        output="screen"
    )

    # Robot state publisher
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[robot_description],
    )

    # RViz
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", PathJoinSubstitution([
            FindPackageShare("my_robot_arm_control"),
            "rviz",
            "robot_control.rviz"
        ])]
    )

    nodes_to_start = [
        robot_controller_node,
        robot_state_publisher_node,
        rviz_node,
    ]

    return LaunchDescription(declared_arguments + nodes_to_start)