
#include "Engine2.h"

using std::stringstream;
using pcl::visualization::PointCloudColorHandlerCustom;
using PCLUtils::addXYZ;
using PCLUtils::subtractXYZ;
using PCLUtils::multiplyXYZ;
using std::endl;

int Engine2::add_object(Point point, bool closest)
{
   RobotPosition objectPosition = getPosition();
   calculate_robot_position(point, objectPosition);

   const float x_delta = objectPosition.x - getPosition().x;
   const float y_delta = objectPosition.y - getPosition().y;
   const float observation_distance = sqrt(x_delta*x_delta + y_delta*y_delta);

   typedef std::vector<WSObject>::iterator   Iterator;

   bool object_exists = false;

   // If we're too far away from an object to do anything, leave it
   if (observation_distance > WSObject::distance_fudge) {
      std::stringstream msg;
      msg << "add_object() : Ignoring the add at (" << objectPosition.x << ", " << objectPosition.y << "). Distance (" << observation_distance
         << ") above fudge factor (" << WSObject::distance_fudge << ")";
      m_logger->log(msg);
   }

   // Else, iterate through the existing objects.
   Iterator it;
   for (it = m_objects->begin(); it != m_objects->end(); it++) {
      
      float x_diff = (*it).x_position - objectPosition.x;
      float y_diff = (*it).y_position - objectPosition.y;
      float distance = sqrt((x_diff*x_diff) + (y_diff*y_diff));

      if ( distance < WSObject::fudge ) {
         std::stringstream ss;
         ss << "add_object(" << objectPosition.x << ", " << objectPosition.y << ") No add - there is an object at (" << (*it).x_position << "," << (*it).y_position << ")." << endl;
         ss << "distance = " << distance << ", fudge = " << WSObject::fudge << ".";              
         m_logger->log(ss);
         break;
      }
   }

   WSObject * update_object;
   WSObject * new_obj = NULL;
   if (it != m_objects->end()) { // We found the object already
      update_object = &(*it);
   } else { // We didnt
      std::stringstream ss;
      ss << "Added object at (" << objectPosition.x << ", " << objectPosition.y << ").";
      m_logger->log(ss);
      new_obj = new WSObject(objectPosition.x, objectPosition.y, observation_distance);
      update_object = new_obj;
   } 

   update_object->point = point;

   // Update R, G, B
   if (closest) {
      update_object->r_display = 0;
      update_object->g_display = 255;
      update_object->b_display = 255;
   } else {
      update_object->r_display = 0;
      update_object->g_display = 255;
      update_object->b_display = 0;
   }


   if (new_obj) {
      m_objects->push_back(*update_object);
      delete new_obj;
   }

   return true;
}

bool Engine2::remove_object(float x, float y) {
   m_logger->log("remove_object not implemented yet.");
   return false;
}


RobotPosition  Engine2::getPosition()
{
   return m_robot->currentPos();
}


bool Engine2::validate_limits(const RobotPosition & pos) 
{
   std::cout << "Implement validateLimits!!" << std::endl;
   return true;
}

bool Engine2::calculate_camera_position(const Point & position, RobotPosition & new_pos)
{
   Point vector_along_floor;

   PCLUtils::subtractXYZ(position, origin_, vector_along_floor);

   float x_amt = PCLUtils::dot_double_normalize(vector_along_floor, m_cal.x_vector);
   float y_amt = PCLUtils::dot_double_normalize(vector_along_floor, m_cal.y_vector);

   new_pos.x += (-1.0 * x_amt * m_cal.x_rpos_amt);
   new_pos.y += (-1.0 * y_amt * m_cal.y_rpos_amt);

   return validate_limits(new_pos);
}

bool Engine2::calculate_robot_position(const Point & position, RobotPosition & new_pos)
{
   Point vector_along_floor;

   PCLUtils::subtractXYZ(position, get_claw_center_projection(), vector_along_floor);

   float x_amt = PCLUtils::dot_double_normalize(vector_along_floor, m_cal.x_vector);
   float y_amt = PCLUtils::dot_double_normalize(vector_along_floor, m_cal.y_vector);

   new_pos.x += (-1.0 * x_amt * m_cal.x_rpos_amt);
   new_pos.y += (-1.0 * y_amt * m_cal.y_rpos_amt);


   return validate_limits(new_pos);
}

void Engine2::calibrate()
{
   // Calibrate the X-axis, if necessary
   if (!m_state.state.x_axis_calibrated) {
      m_state.state.x_axis_calibrated = calibrate_axis(true, m_cal.x_rpos_amt, m_cal.x_vector);
      if (m_state.state.x_axis_calibrated) {
         m_logger->log("Successfully calibrated the x-axis.");
      }
   } else m_logger->log("The x-axis was already calibrated.");


   // Calibrate the Y-axis, if necessary
   if (!m_state.state.y_axis_calibrated) {
      m_state.state.y_axis_calibrated = calibrate_axis(false, m_cal.y_rpos_amt, m_cal.y_vector);
      if (m_state.state.y_axis_calibrated) {
         m_logger->log("Successfully calibrated the y-axis.");
      }
   } else m_logger->log("The y-axis was already calibrated.");


   // If both axes are calibrated, set up the viewer
   if (m_state.state.x_axis_calibrated && m_state.state.y_axis_calibrated)
   {
      m_logger->log("Displaying calibration vectors.");
      axis_view_setup(vp_calibration_axes);
   }
}

void Engine2::load()
{
    using std::cout;
   const std::string calFile = "/Users/amnicholas/Documents/ELEC490/adaptive_grip_recent/data/calibration.dat";
   if (m_cal.fromFile(calFile)) {
      cout << "Succesfully loaded file." << endl;
      cout << m_cal.toString() << endl;
      m_state.state.x_axis_calibrated = true;
      m_state.state.y_axis_calibrated = true;
      axis_view_setup(vp_calibration_axes);

      // TODO MOVE
      setup_claw_line(vp_calibration_axes);

   } else {
      std::cerr << "There was a problem loading the calibration file." << endl;
   }

   if (!m_got_floor_plane) {
      ClusterCloud target; // dummy ClusterCloud to get floor coefficients
      load_and_filter(target);
   }
}

bool Engine2::calibrate_axis(bool x_yb, float & robot_pos_amt, Point & vector)
{

   robot_pos_amt = 100.0;

   ClusterCloud   source, target;

   RobotPosition new_pos = getPosition();
   if (x_yb) {
      new_pos.x = new_pos.x + robot_pos_amt;
   } else {
      new_pos.y = new_pos.y + robot_pos_amt;
   }

   if (!validate_limits(new_pos)) {
      stringstream ss("");
      ss << "calibrate_axis() : Illegal robot position while trying to calibrate.";
      m_logger->log(ss);
      return false;
   }

   // Load source cloud
   load_and_filter(source);
//    add_cloud_to_viewer(source.cloud, "TargetCloud", vp_calibration_clouds, 255, 0, 0);


   // Move to position
   moveTo(new_pos);

   // Load target cloud
   load_and_filter(target);
//    add_cloud_to_viewer(target.cloud, "SourceCloud", vp_calibration_clouds, 0, 0, 255);

   // ClusterDifference
   if (!clusterDifference(source, target, vector)) {
      stringstream ss;
      ss << "calibrate_axis() : clusterDifference() failed. Could not calibrate ";
      ss << (x_yb ? "x" : "y") << " axis." << endl;
      m_logger->log(ss);
      return false;
   }

   return true;
}

// This is only used for calibration, maybe worth moving?
bool Engine2::clusterDifference(ClusterCloud & source, ClusterCloud & target, Point & vector)
{
   source.find_clusters();
   target.find_clusters();

   if (source.size() != 1) {
      stringstream ss;
      ss << "clusterDifference() : Source cloud had " << source.size() << " clusters." << endl;
      ss << "Returning false." << std::endl;
      m_logger->log(ss);
      return false;
   } else if (target.size() != 1) {
      stringstream ss;
      ss << "clusterDifference() : Target cloud had " << target.size() << " clusters." << endl;
      ss << "Returning false." << std::endl;
      m_logger->log(ss);
      return false;
   }

   Point source_cluster = source.clusters->points[0];
   Point target_cluster = target.clusters->points[0];
   PCLUtils::subtractXYZ(target_cluster, source_cluster, vector);

   return true;
}

int   Engine2::locate(bool observe_only) 
{
   m_logger->log("locate() DEPRECATED.");
   return -1;
}

// TODO also update GUI with our last scan location
int Engine2::scan()
{
   // find clusters
   // rank them based on distance to origin
   // move to the closest one

   typedef std::vector<Point>::iterator   Iterator;
   Iterator                               closest_cluster;

   // TODO CHECK THAT WE ARE CALIBRATED!
   if (!m_state.state.x_axis_calibrated || !m_state.state.y_axis_calibrated) {
      m_logger->log("scan() - Not calibrated. Aborting.");
      return -1;
   }

   // Get camera input
   load_raw(current_view);

   // Put it on the viewer
   add_cloud_to_viewer(current_view.cloud, "CurrentCloud", vp_navigation);

   // Filter the floor out
   filter_floor(current_view);

   // Find clusters
   current_view.find_clusters();

   {
      stringstream ss;
      ss << "After find_clusters(): cloud size = " << current_view.cloud->size();
      m_logger->log(ss);
   }

   if (current_view.size() == 0) {
      m_logger->log("scan() - No clusters found.");
      return -1;
   }
   else {
      add_objects(current_view);
      return 0;
   }
}

std::vector<WSObject>::iterator Engine2::get_closest_object()
{
   typedef std::vector<WSObject>::iterator Iterator;
   Iterator    closest = m_objects->begin();
// float shortest_distance = xy_distance(, getPosition());
//
// Get claw projection
   Point claw_center = get_claw_center_projection();
   float shortest_distance = -1;
   for (Iterator i = (m_objects->begin() + 1); i != m_objects->end(); ++i)
   {
      Cluster<Point> current_point_cluster(i->point);
      Point floorPoint = current_point_cluster.get_plane_projection(m_floor_plane);

      float distance = PCLUtils::distance(floorPoint, claw_center);

      if (shortest_distance < 0.0 || distance < shortest_distance) {
         shortest_distance = distance;
         closest = i;
      }
   }
   return closest;
}

// Should accept a pointer to a WSObject instead
//bool Engine2::prepare_grab(Point &centroid)
//
//Use after a scan only
bool Engine2::vantage_point(std::vector<WSObject>::iterator obj)
{
   scan();
   RobotPosition cam_position = getPosition();
   if (!calculate_camera_position(obj->point, cam_position))
   {
      stringstream ss;
      ss << "OBSERVATION Position: " << cam_position;
      m_logger->log(ss);
      m_logger->log("prepare_grab() - the calculated RobotPosition to observe the object is outside of robot limits. Aborting.");
      return false;
   } 

// cam_position.z = properties.vantage_point_claw_height;
   cam_position.z = 0.0;
   moveTo(cam_position);

   return true;
}

bool Engine2::pickup(std::vector<WSObject>::iterator obj)
{
   scan();

   RobotPosition obj_position = getPosition();
   if (!calculate_robot_position(obj->point, obj_position))
   {
      m_logger->log("pickup() - the calcualted RobotPosition is outside of bounds.");
      return false;
   } else {
      moveTo(obj_position);
   }
   return true;
}

void Engine2::load_raw(ClusterCloud & cc) 
{
   cc.cloud.reset(new PointCloud);
// cc.reset();
   m_camera.retrieve();
   *cc.cloud = *cam_cloud_;
// pcl::copyPointCloud(*cam_cloud_, *cc.cloud);

   // Don't filter it
   cc.initialized = true;
}

void Engine2::moveTo(RobotPosition & position)
{
   m_robot->moveTo(position, 0.75);
}

void Engine2::filter_floor(ClusterCloud & cc) 
{
   Mutex dumb_mutex; // TODO un-mutex this
   TwoPlaneFilter<Point>   tp(cc.cloud, &dumb_mutex);
   tp.filter_plane();

   if (!m_got_floor_plane) {
      m_logger->log("load_and_filter() - Updating floor coefficients.");

      m_floor_plane = tp.get_plane_coefficients();
      m_got_floor_plane = true;

      // TODO refactor
      Cluster<Point> originClust(origin_);
      origin_proj_ = originClust.get_plane_projection(m_floor_plane);
   }
}

void Engine2::load_and_filter(ClusterCloud & cc) 
{
   load_raw(cc);
   filter_floor(cc);
}

void Engine2::add_cloud_to_viewer(PointCloud::Ptr cloud, std::string name, int viewport)
{
   viewer->removePointCloud(name);
   viewer->addPointCloud(cloud, name, viewport);
}

void Engine2::add_cloud_to_viewer(PointCloud::Ptr cloud, std::string name, int viewport, int r, int g, int b)
{
   PointCloudColorHandlerCustom<Point>    handler(cloud, r, g, b);
   viewer->removePointCloud(name);
   viewer->addPointCloud(cloud, handler, name, viewport);
}

void Engine2::add_cluster_to_viewer(PointCloud::Ptr clusters, std::string name, int viewport, int r, int g, int b)
{
   viewer->removePointCloud(name);
//    PointCloudColorHandlerCustom<Point>  handler(clusters, r, g, b);
   viewer->addPointCloud(clusters, name, viewport);
   viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 10, name);

}

void Engine2::axis_view_setup(int viewport)
{

   viewer->removeShape("originLine", viewport);
   viewer->addLine(origin_, origin_proj_, "originLine", viewport);

   Point xDifference;
   addXYZ(origin_proj_, m_cal.x_vector, xDifference);
   viewer->removeShape("xRegAxis", viewport);
   viewer->addLine(origin_proj_, xDifference, 214, 0, 244, "xRegAxis", viewport);

   Point yDifference;
   addXYZ(origin_proj_, m_cal.y_vector, yDifference);
   viewer->removeShape("yRegAxis", viewport);
   viewer->addLine(origin_proj_, yDifference, 255, 165, 0, "yRegAxis", viewport);
}

/* Constructor */
Engine2::Engine2(pcl::visualization::PCLVisualizer::Ptr vis,
      Logger * logger) :
   cam_cloud_(new PointCloud()),
   dummy_mutex_(new Mutex()),
   m_camera(cam_cloud_, dummy_mutex_),
   m_robot(new RobotExt("/dev/cu.usbserial")),
   m_logger(logger)
   
{
   m_got_floor_plane = false;

   m_objects = new std::vector<WSObject>();

   m_state.state.x_axis_calibrated = false;

   // Origin
   origin_.x = 0; origin_.y = 0; origin_.z = 0;

   if (vis) viewer = vis;

   // Setup viewports
   /* 4 Viewports 
   viewer->createViewPort(0.0, 0.0, 0.5, 0.5, vp_calibration_clouds);
   viewer->createViewPort(0.5, 0.0, 1.0, 0.5, vp_calibration_axes);
   viewer->createViewPort(0.0, 0.5, 0.5, 1.0, vp_navigation);
   viewer->createViewPort(0.5, 0.5, 1.0, 1.0, vp_workspace); */

   viewer->createViewPort(0.0, 0.0, 1.0, 0.5, vp_calibration_axes);
   viewer->createViewPort(0.0, 0.5, 1.0, 1.0, vp_navigation);
}

bool Engine2::add_objects(ClusterCloud & cloud)
{
   RobotPosition currentPos = getPosition();

   typedef     std::vector<Point>::iterator  Iterator;

   std::vector<Point>::iterator closest_cluster;

   float       shortest_distance = -1.0;

   stringstream ss;
   ss << "Processing " << cloud.size() << " objects." << endl;
   m_logger->log(ss);

   // TODO reset the colors of all WSObjects, e.g. to default, before we overwrite them

   // Find the point with the shortest distance.
   for (Iterator i = cloud.begin(); i != cloud.end(); ++i)
   {
      Cluster<Point> current_point_cluster(*i);
      Point floorPoint = current_point_cluster.get_plane_projection(m_floor_plane);

      // Get the norm distance between floorPoint and originProjection
      float distance = PCLUtils::distance(floorPoint, origin_proj_);

      if (shortest_distance < 0 || distance < shortest_distance) {
         shortest_distance = distance;
         closest_cluster = i;
      } 
   } // for

   for (Iterator i = cloud.begin(); i != cloud.end(); ++i)
   {
//   OLD --   RobotPosition pos = currentPos;
//   OLD --   calculate_robot_position(*i, pos);
//   OLD --   if (i == closest_cluster) {
//   OLD --      i->r = 0; i->g = 255; i->b = 255;
//   OLD --      add_object(pos.x, pos.y, currentPos, CLOSEST_OBJECT);
//   OLD --   } else {
//   OLD --      i->r = 0; i->g = 255; i->b = 0;
//   OLD --      add_object(pos.x, pos.y, currentPos, DEFAULT_OBJECT);
//   OLD --   }
      if (i == closest_cluster) {
         i->r = 0; i->g = 255; i->b = 255;
         add_object(*i, true);
      } else {
         i->r = 0; i->g = 255; i->b = 0;
         add_object(*i, false);
      }
   }

// get_closest_object

   add_cluster_to_viewer(cloud.clusters, "CurrentCloud_Clusters", vp_navigation);

   return true;
}

int
Engine2::move_to_object(int index)
{
   // Check if the index is valid
   if (index < 0 || index >= m_objects->size()) {
      std::stringstream emsg;
      emsg << "Engine2::move_to_object() - Illegal index (" << index << ") provided. m_objects->size() = "
         << m_objects->size() << ".";
      m_logger->log(emsg);
      return FAILURE;
   }

// RobotPosition new_pos = getPosition();
// new_pos.x = (*m_objects)[index].x_position;
// new_pos.y = (*m_objects)[index].y_position;
// 
// moveTo(new_pos);

   // This will move us to the nearest object
// int locate_result = locate(false);
// locate();

// std::stringstream msg;
// msg << "Engine2::move_to_object() - DEBUG: locate() returned << " << locate_result << ".";
// m_logger->log(msg);

   std::vector<WSObject>::iterator object = m_objects->begin();
   std::advance(object, index);

   DMSG("move_to_object(): moving to vantage point.")
   {
      std::stringstream msg;
      msg << "vantage-pointing to Object [" << object->id << "].";
      m_logger->log(msg);
   }

   RobotPosition initial = getPosition();
   initial.x = object->x_position + 100.0;
   initial.y = object->y_position;
   moveTo(initial);

// vantage_point(object);

// DMSG("Preparing to re-scan.")
// scan();

// DMSG("Preparing to pick up.")
   {
      std::stringstream msg;
      msg << "Picking up Object [" << object->id << "].";
      m_logger->log(msg);
   }

   pickup(object);

   // Re-load the point cloud

   return SUCCESS;

}

void
Engine2::setup_claw_line(int viewport)
{
   // TODO should check that we have calibration before doing this
   viewer->removeShape("OriginLine", viewport);
   viewer->addLine(origin_, origin_proj_, "OriginLine", viewport);

   move_claw_line(0.0, 0.0, viewport);

}

void
Engine2::clear_objects()
{
   m_logger->log("Erasing all objects in workspace.");
   m_objects->clear();
}


Engine2::Point Engine2::get_claw_center_projection()
{
   Point claw_base, x_vec, y_vec;
   Point combined_vec;
   
   multiplyXYZ(m_cal.x_vector, m_cal.x_adj_amt, x_vec);
   multiplyXYZ(m_cal.y_vector, m_cal.y_adj_amt, y_vec);

   addXYZ(x_vec, y_vec, combined_vec);
   addXYZ(origin_, combined_vec, claw_base);
   
   Cluster<Point> claw_center_clust(claw_base);

   // TODO make the floor plane a member of the calibration class.
   Point claw_center_proj = claw_center_clust.get_plane_projection(m_floor_plane);

   return claw_center_proj;
}


void 
Engine2::move_claw_line(float x_amount, float y_amount, int viewport)
{
   Point claw_base;
   Point x_vec, y_vec;
   Point combined_vec;

   m_cal.x_adj_amt = x_amount;
   m_cal.y_adj_amt = y_amount;

   multiplyXYZ(m_cal.x_vector, x_amount, x_vec);
   multiplyXYZ(m_cal.y_vector, y_amount, y_vec);

   addXYZ(x_vec, y_vec, combined_vec);
   addXYZ(origin_, combined_vec, claw_base);

   

// {
//    std::stringstream debug;
//    debug << "claw_base.x = " <<claw_base.x << " "
//    << "claw_base.y = " << claw_base.y << " "
//    << "claw_base.z = " << claw_base.z;
//    m_logger->log(debug);
// }
   
   Cluster<Point> claw_center_clust(claw_base);
   Point claw_center_proj = claw_center_clust.get_plane_projection(m_floor_plane);

// {
//    std::stringstream debug;
//    debug << "claw_center_proj.x = " << claw_center_proj.x << " "
//    << "claw_center_proj.y = " << claw_center_proj.y << " "
//    << "claw_center_proj.z = " << claw_center_proj.z;
//    m_logger->log(debug);
// }

   ClusterCloud claw_proj_cloud;
   claw_proj_cloud.cloud->push_back(claw_base);
   claw_proj_cloud.cloud->push_back(claw_center_proj);
   add_cluster_to_viewer(claw_proj_cloud.cloud, "claw_proj_cloud", viewport, 255, 0, 0);

   viewer->removeShape("ClawLine");
   viewer->addLine(claw_base, claw_center_proj, 255, 0, 0, "ClawLine", viewport);
}
