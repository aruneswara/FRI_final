#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <locale>  
#include <bits/stdc++.h>
#include <experimental/filesystem>
#include <chrono>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>
#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <sensor_msgs/image_encodings.h>
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include <move_base_msgs/MoveBaseAction.h>

std::unordered_set<std::string> dictionary;
std::experimental::filesystem::v1::__cxx11::directory_entry f;
void setWords()
{
    std::fstream file;
    std::string word, filename = "/home/arun/catkin_ws/src/final/src/words.txt";
    file.open(filename);
    if(!file.is_open())
    {
        std::cout << "error openning file" << std::endl;
        return;
    }
    while(file >> word)
        dictionary.insert(word); 
}

void checkWords(std::vector<std::string> &v)
{
    std::vector<std::string> out;
    for(int i = 0; i<v.size(); i++)
    {
        std::unordered_set<std::string>::const_iterator got = dictionary.find (v[i]);
        if(got!=dictionary.end())
        {
            out.push_back(v[i]);
        }
    }
    v =  out;
}
 
std::string lower(std::string str)
{
    std::locale loc;
    for (std::string::size_type i=0; i<str.length(); ++i)
        str[i] = std::tolower(str[i],loc);
    return str;
}

class Final
{
    protected:
        ros::Subscriber _sub;
        geometry_msgs::Pose _robot;
    public:
        Final(ros::NodeHandle &n)
        {
            _sub = n.subscribe("/amcl_pose",100, &Final::cb, this);
        };

        void cb(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& msg) {
            _robot = msg->pose.pose;
            std::string imagePath = "/home/arun/catkin_ws/src/final/images";
            cv::Mat image;

            std::vector<std::tuple<std::vector<std::string>, std::string, int, int>> text;

            tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
            api->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
            api->SetPageSegMode(tesseract::PSM_AUTO);
            api->SetVariable("debug_file", "tesseract.log");

            auto start = std::chrono::steady_clock::now();
            std::string filepath = f.path().string();
            //std::cout << "Detecting text in " << filepath << std::endl;

            image = cv::imread(filepath, 1);

            api->SetImage(image.data, image.cols, image.rows, 3, image.step);
            std::string outText = api->GetUTF8Text();
            std::vector<std::string> words;
            
            std::stringstream ss(outText);
            std::string buf;
            while(ss>>buf)
                words.push_back(lower(buf));

            checkWords(words);
            text.push_back(std::make_tuple(words,filepath,_robot.position.x,_robot.position.y));
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> diff = end - start;
            std::cout << "Computation time: " << diff.count() << "ms" << std::endl;
            
            api->End();
            for(auto t: text)
            {
                std::vector<std::string> text = std::get<0>(t);
                std::string file = std::get<1>(t);
                int x = std::get<2>(t);
                int y = std::get<3>(t);
                for(auto x: text)
                    std::cout << x << " ";
                std::cout << std::endl;
                std::cout << "File: " << file << std::endl << "x: " << x << " --- y: " << y << std::endl << std::endl;
            }
        }
           
};



int main(int argc, char**argv) {
    setWords();
    ros::init(argc, argv, "final_node");
    ros::NodeHandle n;
    Final f(n);
    for (const auto &fn : std::experimental::filesystem::directory_iterator(imagePath)) {
    {
        f = fn;
        ros::spinOnce();
    }
    
    return 0;
}