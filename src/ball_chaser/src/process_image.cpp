#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("drive the robot in the specified direction");

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z; 
    if (!client.call(srv))
        ROS_ERROR("Failed to call service safe_move");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    //ROS_INFO("image has step:%1.2f, and row:%1.2f, and collum:%1.2f",(float)img.step, (float)img.height, (float)img.width); 
    //TOKNOW: img.step=2400 , img.width= 800, img.height = 800

    float white_pixel_location;
    bool white_image_identified = false; 
    int increment=3; //to change the value for suitability
    int precission=6; //to change the value for precission
     for (int i = 0; i < img.height * img.step ; i +=increment) {
         if(img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel ){
             if(img.data[i+precission] == white_pixel){
                 if(img.data[i-precission] == white_pixel){
                     white_pixel_location = i % img.step;
                     white_image_identified=true;
                     break;
                 }
             }
         }
    }
    // get white pixel location
    float image_position = white_pixel_location / img.step;
    float left_side=0.35;    
    float center=0.65;
    float right_side=1;

    if(white_image_identified == true){
        if(image_position < left_side){ 
            drive_robot(0.2, 0.5); 
        }
        else if (image_position >= left_side && image_position < center){ 
            drive_robot(0.5, 0); 
        }
        else if(image_position >= center && image_position < right_side){ 
            drive_robot(0.2, -0.5); 
        }
        ROS_INFO("image found at :%1.2f, found in pixel :%1.2f", (float)image_position, (float) white_pixel_location);
    }else{ 
        drive_robot(0, 0); 
        ros::Duration(0.5).sleep();
  }
    }

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
