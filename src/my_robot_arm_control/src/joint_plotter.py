#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
import matplotlib.pyplot as plt
import threading
import time
from collections import deque

class JointPlotter(Node):
    def __init__(self):
        super().__init__('joint_plotter')
        
        # Data storage
        self.time_data = deque(maxlen=200)
        self.actual_data = deque(maxlen=200)
        self.target_data = deque(maxlen=200)
        
        # Current values
        self.current_actual = 0.0
        self.current_target = 0.0
        self.start_time = time.time()
        
        # Subscribers
        self.actual_sub = self.create_subscription(
            JointState, '/joint_states', self.actual_callback, 10)
        self.target_sub = self.create_subscription(
            JointState, '/joint_commands', self.target_callback, 10)
        
        print("Joint plotter started. Send commands to see the plot!")
        
    def actual_callback(self, msg):
        if len(msg.position) > 0:
            self.current_actual = msg.position[1]  # Joint 0
            current_time = time.time() - self.start_time
            
            self.time_data.append(current_time)
            self.actual_data.append(self.current_actual)
            self.target_data.append(self.current_target)
            
            # Print values for debugging
            print(f"Time: {current_time:.1f}s, Target: {self.current_target:.3f}, Actual: {self.current_actual:.3f}")
    
    def target_callback(self, msg):
        if len(msg.position) > 0:
            self.current_target = msg.position[1]  # Joint 1
            print(f"New target received: {self.current_target:.3f}")
    
    def plot_data(self):
        plt.ion()  # Interactive mode
        fig, ax = plt.subplots()
        
        while rclpy.ok():
            if len(self.time_data) > 1:
                ax.clear()
                ax.plot(list(self.time_data), list(self.actual_data), 'b-', label='Actual Joint 1', linewidth=2)
                ax.plot(list(self.time_data), list(self.target_data), 'r--', label='Target Joint 1', linewidth=2)
                ax.legend()
                ax.set_xlabel('Time (s)')
                ax.set_ylabel('Joint Angle (rad)')
                ax.set_title('Joint 0 Position Tracking')
                ax.grid(True)
                
                plt.pause(0.1)
        
        plt.ioff()

def main():
    rclpy.init()
    plotter = JointPlotter()
    
    # Start plotting in a separate thread
    plot_thread = threading.Thread(target=plotter.plot_data)
    plot_thread.daemon = True
    plot_thread.start()
    
    try:
        rclpy.spin(plotter)
    except KeyboardInterrupt:
        print("Shutting down...")
    finally:
        rclpy.shutdown()

if __name__ == '__main__':
    main()