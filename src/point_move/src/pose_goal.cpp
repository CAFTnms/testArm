#include<ros/ros.h>
#include <moveit/move_group_interface/move_group_interface.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Pose.h>
#include <tf/transform_datatypes.h>
#include <moveit/kinematic_constraints/utils.h>
#include <moveit_msgs/PlanningScene.h>
#include <moveit/robot_model_loader/robot_model_loader.h>
#include <moveit/planning_interface/planning_interface.h>
#include <moveit/planning_scene/planning_scene.h>
#include <tf2/LinearMath/Quaternion.h>
#include <sensor_msgs/JointState.h>
#include <iostream>
#include <moveit_msgs/DisplayRobotState.h>
#include <moveit_msgs/DisplayTrajectory.h>

#include <moveit_msgs/AttachedCollisionObject.h>
#include <moveit_msgs/CollisionObject.h>


double x_input, y_input, z_input;
double roll, pitch, yaw;


class Localization {

public:
    double pos;
    void servo(const sensor_msgs::JointState::ConstPtr&  arm_step);
};


void Localization::servo(const sensor_msgs::JointState::ConstPtr& arm_step){
  
  pos = arm_step->position[0];
  
}

using namespace std;

int main(int argc, char** argv)
{

  //int i = 0;
  
  ros::init(argc, argv, "pose_goal");
  ros::NodeHandle node_handle;
  Localization localization;
  ros::Subscriber sub = node_handle.subscribe<sensor_msgs::JointState> ("joint_states",1, &Localization::servo,&localization);
  ros::AsyncSpinner spinner(1);
  spinner.start();
  ros::Rate rate(0.5);
  
  moveit::planning_interface::MoveGroupInterface move_group_interface("arm");
  moveit::planning_interface::MoveGroupInterface move_group_interface_gripper("hand");
  const robot_state::JointModelGroup *joint_model_group =
    move_group_interface.getCurrentState()->getJointModelGroup("arm");
  
  moveit::planning_interface::MoveGroupInterface::Plan my_plan;
  bool success;
  //rest position
  move_group_interface.setJointValueTarget(move_group_interface.getNamedTargetValues("rest"));
  success = (move_group_interface.plan(my_plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  if (success)
  {
    ROS_INFO("[point_move/pose_goal] Planning OK. Proceeding.");
  }
  else
  {
    ROS_WARN("[point_move/pose_goal] Planning failed. Shutting down.");
    ros::shutdown();
    return 0;
  }
  move_group_interface.move();
  
  for(int i=0;i<5;i++){
  
  //open gripper
  moveit::planning_interface::MoveGroupInterface::Plan my_plan_gripper;
  move_group_interface_gripper.setJointValueTarget(move_group_interface_gripper.getNamedTargetValues("open"));
  success = (move_group_interface_gripper.plan(my_plan_gripper) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  move_group_interface_gripper.move();

  //move to given coordinate
  //move_group_interface.setEndEffectorLink("Link_04");
  move_group_interface.setPlanningTime(10.0);
  move_group_interface.setGoalJointTolerance(1e-4);
  move_group_interface.setGoalPositionTolerance(1e-4);
  move_group_interface.setGoalOrientationTolerance(1e-3);
  
  //cout << "Enter Value x";
  //cin >> x_input;
  //cout << "Enter Value y";
  //cin >> y_input;
  //cout << "Enter Value z";
  //cin >> z_input;
  
  geometry_msgs::PoseStamped current_pose;
  current_pose = move_group_interface.getCurrentPose();
  geometry_msgs::Pose target_pose;
  target_pose.position.x = -0.0741837;
  target_pose.position.y = 0.0353531;
  target_pose.position.z = -0.584249;
  tf2::Quaternion myQuaternion;
  myQuaternion.setRPY( 1.5708, 0, 0); 
  ROS_INFO_STREAM("x: " << myQuaternion.getX() << " y: " << myQuaternion.getY() << " z: " << myQuaternion.getZ() << " w: " << myQuaternion.getW());
  myQuaternion= myQuaternion.normalize();
  target_pose.orientation.w = myQuaternion.getW()  ;
  target_pose.orientation.x = myQuaternion.getX()  ;
  target_pose.orientation.y = myQuaternion.getY() ;
  target_pose.orientation.z = myQuaternion.getZ() ;
  
  move_group_interface.setApproximateJointValueTarget(target_pose);
  
  
  success = (move_group_interface.plan(my_plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  
  if (success)
  {
    ROS_INFO("[point_move/pose_goal] Planning OK. Proceeding.");
  }
  else
  {
    ROS_WARN("[point_move/pose_goal] Planning failed. Shutting down.");
    ros::shutdown();
    return 0;
  }
  

  move_group_interface.execute(my_plan);
  ros::Duration(3).sleep();
    
  //close gripper
  move_group_interface_gripper.setJointValueTarget(move_group_interface_gripper.getNamedTargetValues("close"));
  success = (move_group_interface_gripper.plan(my_plan_gripper) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  move_group_interface_gripper.move();
  ros::Duration(0.5).sleep();
  ros::spinOnce();
  ROS_INFO("pos:%.2f", localization.pos); 
  rate.sleep();
  
  //drop position
  move_group_interface.setJointValueTarget(move_group_interface.getNamedTargetValues("drop"));
  success = (move_group_interface.plan(my_plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  if (success)
  {
    ROS_INFO("[point_move/pose_goal] Planning OK. Proceeding.");
  }
  else
  {
    ROS_WARN("[point_move/pose_goal] Planning failed. Shutting down.");
    ros::shutdown();
    return 0;
  }
  move_group_interface.move();
  ros::Duration(1.5).sleep();
  
  //open gripper
  move_group_interface_gripper.setJointValueTarget(move_group_interface_gripper.getNamedTargetValues("open"));
  success = (move_group_interface_gripper.plan(my_plan_gripper) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  move_group_interface_gripper.move();
  ros::Duration(1).sleep();
  
  
  //i++; //counter
  }

  
  //rest position
  move_group_interface.setJointValueTarget(move_group_interface.getNamedTargetValues("rest"));
  success = (move_group_interface.plan(my_plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);
  if (success)
  {
    ROS_INFO("[point_move/pose_goal] Planning OK. Proceeding.");
  }
  else
  {
    ROS_WARN("[point_move/pose_goal] Planning failed. Shutting down.");
    ros::shutdown();
    return 0;
  }
  move_group_interface.move();
  
  
  ros::shutdown();
  return 0;


}
