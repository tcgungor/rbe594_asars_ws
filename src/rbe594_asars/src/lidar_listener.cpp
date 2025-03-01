#include "ros/ros.h"
#include "tf/transform_listener.h"
#include "sensor_msgs/PointCloud2.h"
#include "tf/message_filter.h"
#include "tf/transform_broadcaster.h"
#include "message_filters/subscriber.h"
#include "laser_geometry/laser_geometry.h"
#include "pcl/point_cloud.h"
#include "pcl/filters/voxel_grid.h"
#include "pcl/io/pcd_io.h"
#include "pcl_conversions/pcl_conversions.h"
#include "pcl/filters/statistical_outlier_removal.h"
#include "csignal"
#include "cstdlib"
#include "std_msgs/String.h"


class LaserScanToPointCloud {

public:

  ros::NodeHandle n_;
  laser_geometry::LaserProjection projector_;
  tf::TransformListener tf_;
  message_filters::Subscriber<sensor_msgs::LaserScan> laser_sub_;
  tf::MessageFilter<sensor_msgs::LaserScan> laser_notifier_;
  ros::Publisher scan_pub_;
  pcl::PCLPointCloud2 full_cloud;
  

  LaserScanToPointCloud(ros::NodeHandle n): 
    n_(n),
    laser_sub_(n_, "/scan", 10),
    laser_notifier_(laser_sub_, tf_, "world", 10)
  {
    laser_notifier_.registerCallback( boost::bind(&LaserScanToPointCloud::scanCallback, this, _1));
    laser_notifier_.setTolerance(ros::Duration(0.01));
    scan_pub_ = n_.advertise<sensor_msgs::PointCloud2>("/my_cloud",1);
  }

  void scanCallback(const sensor_msgs::LaserScan::ConstPtr& scan_in){
    sensor_msgs::PointCloud2 cloud;
//    ROS_INFO("Scanning");
    
    try {
        projector_.transformLaserScanToPointCloud("world", *scan_in, cloud, tf_);
    }
    catch (tf::TransformException& e) {
        std::cout << e.what();
        return;
    }

    pcl::PCLPointCloud2 newPoints;
    pcl_conversions::toPCL(cloud, newPoints);
    pcl::concatenate(full_cloud, newPoints, full_cloud);
    pcl_conversions::fromPCL(full_cloud, cloud);

    scan_pub_.publish(cloud);

  }

  void exportVoxelGrid(){
    
    ROS_INFO("Saving map pcd...");
    sensor_msgs::PointCloud2 cloud;
    pcl_conversions::fromPCL(full_cloud, cloud);

    pcl::PCLPointCloud2::Ptr cloud2(new pcl::PCLPointCloud2());
    pcl::PCLPointCloud2::Ptr cloud_filtered(new pcl::PCLPointCloud2());
    pcl_conversions::toPCL(cloud, *cloud2);

    pcl::StatisticalOutlierRemoval<pcl::PCLPointCloud2> sor;
    sor.setInputCloud(cloud2);
    sor.setMeanK(50);
    sor.setStddevMulThresh(1.0);
    sor.filter(*cloud_filtered);

    pcl::PCLPointCloud2::Ptr cloud_voxelized(new pcl::PCLPointCloud2());
    pcl::VoxelGrid<pcl::PCLPointCloud2> voxelizer;
    voxelizer.setInputCloud(cloud_filtered);
    voxelizer.setLeafSize(0.5f, 0.5f, 0.5f); // leaf size of 0.5m
    voxelizer.filter(*cloud_voxelized);

    pcl::PCDWriter writer;
    writer.write("map_grid.pcd", cloud_voxelized, Eigen::Vector4f::Zero(), Eigen::Quaternionf::Identity(), false);
    
  }

};


LaserScanToPointCloud *lstopc;

void signal_handler(int signal) {
  lstopc->exportVoxelGrid();
  std::exit(signal);
}

void kill_handler(const std_msgs::String::ConstPtr& msg) {
  ROS_INFO("Received message: [%s]", msg->data.c_str());
  if (strcmp(msg->data.c_str(), "kill") == 0) {
    lstopc->exportVoxelGrid();
  }
}

int main(int argc, char** argv){
  
  ros::init(argc, argv, "my_scan_to_cloud");
  ros::NodeHandle n;
  ROS_INFO("Starting...");
  lstopc = new LaserScanToPointCloud(n);

  ros::Subscriber sub = n.subscribe("mapping_status", 100, kill_handler);
  std::signal(SIGINT, signal_handler);

  ros::spin();
  
  return 0;
}