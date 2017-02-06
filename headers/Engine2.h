#ifndef ENGINE2__H
#define ENGINE2__H


#include "PCLUtils.h"
#include "RobotExt.h"
#include "TwoPlaneFilter.h"
#include "Mutex.h"
#include "Camera.h"
#include "common_defs.h"
#include "Cluster.h"
#include "ClusterFinder.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include "inst/pcl_visualizer.h"
#include "CCloud.h"
#include "Calibration.h"
#include "WSObject.h"




using std::stringstream;
using pcl::visualization::PointCloudColorHandlerCustom;
using PCLUtils::addXYZ;
using PCLUtils::subtractXYZ;
using std::endl;

class Engine2 {
public: 

   typedef pcl::PointXYZRGBA        Point;
   typedef pcl::PointCloud<Point>   PointCloud;

   typedef CCloud ClusterCloud;

// private:

   PointCloud::Ptr   cam_cloud_;
   Mutex *           dummy_mutex_;

   // VIEWPORTS
   int      vp_calibration_clouds;
   int      vp_calibration_axes;
   int      vp_navigation;
   int      vp_workspace;

   Point                            origin_;
   Point                            origin_proj_;


public:

   ClusterCloud current_view;

   // TODO make public and make a const getter
   pcl::visualization::PCLVisualizer::Ptr viewer;

   std::vector<WSObject> * m_objects;


   typedef struct {
      bool x_axis_calibrated;
      bool y_axis_calibrated;
//    bool robot_homed;
   } state_t;

   typedef union {
      state_t  state;
      char     mask;
   }  ustate_t;

// const int CLOSEST_OBJECT = 40;
// const int DEFAULT_OBJECT = 50;
   #define CLOSEST_OBJECT 50
   #define DEFAULT_OBJECT 40

// int add_object(float x, float y, const RobotPosition & observed_position, int objType = DEFAULT_OBJECT);
   int add_object(Point, bool);
   bool remove_object(float x, float y);


   RobotExt *                       m_robot;
   bool                             m_got_floor_plane;
   Eigen::Vector4f                  m_floor_plane;
   ustate_t                         m_state;
   std::vector<Point>               m_clusters;
   Camera<Point>                    m_camera;
   Logger *                         m_logger;


   Calibration                            m_cal;

   RobotPosition  getPosition();


   // TODO Move to its own file
   bool validate_limits(const RobotPosition & pos) ;

   // move to calibraiton
   bool calculate_robot_position(const Point & position, RobotPosition & new_pos);
   bool calculate_camera_position(const Point & position, RobotPosition & new_pos);

   void calibrate();

   void load();

   bool calibrate_axis(bool x_yb, float & robot_pos_amt, Point & vector);

   // This is only used for calibration, maybe worth moving?
   bool clusterDifference(ClusterCloud & source, ClusterCloud & target, Point & vector);
   
   int   locate(bool observe_only);
   
   void load_raw(ClusterCloud & cc);

   virtual void moveTo(RobotPosition & position);

   void filter_floor(ClusterCloud & cc);
   
   void load_and_filter(ClusterCloud & cc);

   // internal
   void add_cloud_to_viewer(PointCloud::Ptr cloud, std::string name, int viewport); // move to viewer

   // internal
   void add_cloud_to_viewer(PointCloud::Ptr cloud, std::string name, int viewport, int r, int g, int b); // move to viewer

   // internal
   void add_cluster_to_viewer(PointCloud::Ptr clusters, std::string name, int viewport, int r=255, int g=255, int b=255); // move to viewer

   // internal
   void axis_view_setup(int viewport); // move to viewer

   RobotPosition currentPos();

   // MOVE THESE TO VIEWER CLASS
   Point claw_center;
   void setup_claw_line(int viewport); // move to viewer
   void move_claw_line(float, float, int); // move to viewer


   /* Constructor */
   Engine2(
         pcl::visualization::PCLVisualizer::Ptr vis   = pcl::visualization::PCLVisualizer::Ptr(new pcl::visualization::PCLVisualizer("viewer")),
         Logger * logger                              = new Logger());

   bool add_objects(ClusterCloud & cloud);

   int  move_to_object(int);

   void clear_objects();

   // TODO do this more elegantly
   Point    get_claw_center_projection();

   bool prepare_grab(std::vector<WSObject>::iterator);

   virtual int scan();

   std::vector<WSObject>::iterator get_closest_object();

   bool vantage_point(std::vector<WSObject>::iterator);
   bool pickup(std::vector<WSObject>::iterator);

public:
   // Just in case..
   EIGEN_MAKE_ALIGNED_OPERATOR_NEW
   
};

#endif // ENGINE2__H
