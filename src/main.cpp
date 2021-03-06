///////////////////////////////////////////////////////////////////////////////
// main.cpp - Main for DROPS - Dynamics Realtime Obstacle Pathing System
//Copyright (C) 2015  Christopher Newport University
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
///////////////////////////////////////////////////////////////////////////////

#include "main.hpp"
#include "communication.hpp"
#include "plan.hpp"
#include "util.hpp"

#include <signal.h>

#include <chrono>

#include <boost/asio.hpp>

// C++ REST SDK
#include <cpprest/http_client.h>

const std::string CONFIG_FILENAME = "./src/communicator_config.txt";

using namespace web;
using namespace web::http;
using namespace web::http::client;

void print_env(env_data_t my_env_data, point_char_map moving_obs, std::vector<sbpl_xy_theta_pt_t> path)
{
    std::cout << "Grid"  << std::endl << std::endl;
    point_char_map path_points = point_char_map();
    for (unsigned int i = 0; i < path.size(); i++) {
        int pt_x = (int)path.at(i).x;
        int pt_y = (int)path.at(i).y;
        int pt_ang = ((int)((RAD_TO_DEG(path.at(i).theta) + 22.5) / 45)) % 8;
        std::pair<int, int> point = std::pair<int, int>(pt_x, pt_y);
        //Points 0-7 0 for staight, 1 for angle, etc.
        if (path_points.count(point) == 1) {
            int existing_ang = path_points.at(point);
            pt_ang = (pt_ang + existing_ang) / 2;
        }
        path_points[point] = pt_ang;
    }
    for(int i = 0; i < my_env_data.height; i++) {
        for(int j = 0; j < my_env_data.width; j++) {
            std::pair<int, int> point = std::pair<int, int>(j, i);
            if(j == my_env_data.start_x && i == my_env_data.start_y) {
                std::cout << "S";
            } else if (j == my_env_data.end_x && i == my_env_data.end_y) {
                std::cout << "G";
            } else if (path_points.count(point) == 1) {
                switch (path_points.at(point)) {
                case 0:
                case 4:
                    std::cout << "-";
                    break;
                case 1:
                case 5:
                    std::cout << "\\";
                    break;
                case 2:
                case 6:
                    std::cout << "|";
                    break;
                case 3:
                case 7:
                    std::cout << "/";
                    break;
                }
            } else if (moving_obs.count(point) == 1) {
                unsigned char obs_val = moving_obs.at(point);
                if (obs_val == 255) {
                    std::cout << "O";
                } else if(obs_val == 0) {
                    std::cout << " ";
                } else {
                    std::cout << (obs_val / 26);
                }
            } else if(my_env_data.grid_2d[j + i * my_env_data.width] == 0) {
                std::cout << " ";
            } else if (my_env_data.grid_2d[j + i * my_env_data.width] < 255) {
                std::cout << (my_env_data.grid_2d[j + i * my_env_data.width] / 26);
            } else {
                std::cout << "O";
            }
        }
        std::cout << std::endl;
    }
}

void print_evn_data(env_data_t my_env_data)
{
    std::cout << "height:" << my_env_data.height << std::endl;
    std::cout << "width" << my_env_data.width << std::endl;
    std::cout << "start_x" << my_env_data.start_x << std::endl;
    std::cout << "start_y" << my_env_data.start_y << std::endl;
    std::cout << "start_theta" << my_env_data.start_theta << std::endl;
    std::cout << "end_x" << my_env_data.end_x << std::endl;
    std::cout << "end_y" << my_env_data.end_y << std::endl;
    std::cout << "end_theta" << my_env_data.end_theta << std::endl;
}

void print_env_const(env_constants_t my_env_const)
{
    std::cout << std::endl << "Env Constatnts" << std::endl;
    std::cout << "obs_thresh: " << +my_env_const.obs_thresh << std::endl;
    std::cout << "cost_inscribed_thresh: " << +my_env_const.cost_inscribed_thresh << std::endl;
    std::cout << "cost_possibly_circumscribed_thresh: " << my_env_const.cost_possibly_circumscribed_thresh << std::endl;
    std::cout << "est_velocity: " << my_env_const.est_velocity << std::endl;
    std::cout << "timetoturn45degs: " << my_env_const.timetoturn45degs << std::endl;
    std::cout << "cellsize_m: " << my_env_const.cellsize_m << std::endl << std::endl;

}

void print_path(std::vector<sbpl_xy_theta_pt_t> path)
{
    for (unsigned int i = 0; i < path.size(); i++) {
        printf("%3.3f %3.3f %3.3f\n", path.at(i).x, path.at(i).y, RAD_TO_DEG(path.at(i).theta));
    }
}

void print_moving_obs_points(point_char_map mov_obs)
{
    std::cout << "Printing Moving Obstacles" << std::endl;
    for (auto obs : mov_obs) {
        std::cout << obs.first.first << ", " << obs.first.second << " " << (int)obs.second << std::endl;
    }

}

void signal_handler(int s)
{
    std::printf("Caught signal %d\n", s);
    exit(1);
}

int main(int argc, char *argv[])
{
    // Setup signal interrupt
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);


    communicator my_communicator;
    if (my_communicator.import_config(CONFIG_FILENAME) != 0) {
        std::cout << "Error with config: EXITING" << std::endl;
        return 1;
    }

    auto start = std::chrono::system_clock::now();
    my_communicator.update_data();
    std::cout << "Waiting for first data from server" << std::endl;
    while(my_communicator.update_in_progress()) {
        usleep(10);
    }
    if(!my_communicator.is_updated()) {
        std::cout << "Failed to get grid data from server!" << std::endl;
        return 1;
    }
    auto end = std::chrono::system_clock::now();
    auto elapsed = end - start;

    env_data_t my_env_data = my_communicator.get_env_data();
    env_constants_t my_env_const = my_communicator.get_const_data();
    point_char_map moveing_obs_pts = my_communicator.get_updated_points();

    std::cout << "Height:   " << my_env_data.height << std::endl;
    std::cout << "Width:    " << my_env_data.width << std::endl;
    std::cout << "Communicaiton Time(ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;

#ifdef _DEBUG
    print_evn_data(my_env_data);
    print_env_const(my_env_const);

    std::cout << "Creating Planner" << std::endl;
#endif

    start = std::chrono::system_clock::now(); //Timing start

    //Create a planner object
    Planner my_planner;
    {
        //Lock the grid, then intialize the planner
        std::unique_lock<std::mutex> env_grid_lock = my_communicator.get_lock_env_grid_2d();
#ifdef _DEBUG
        std::cout << "Initialize Planner" << std::endl;
#endif
        my_planner.initialize(my_env_data, my_env_const);
    }

    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "Planner Init Time(ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;


    //Update the planner with the moving obstacles
#ifdef _DEBUG
    std::cout << "Update Planner" << std::endl;
#endif
    start = std::chrono::system_clock::now(); //Timing start
    my_planner.update_grid_points(moveing_obs_pts);

    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "Update Planner Time(ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;

    //Plan!
#ifdef _DEBUG
    std::cout << "Plan" << std::endl;
#endif
    start = std::chrono::system_clock::now(); //Timing start
    int has_path = my_planner.plan();

    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "Planning Time(ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;


    if(has_path) {
        std::cout << "Has path" << std::endl;
    } else {
        std::cout << "NO PATH FOUND"  << std::endl;
    }

#ifdef _DEBUG
    if(has_path) {
        //Print the path to stdout
        //print_path(my_planner.get_path());
        print_env(my_env_data, moveing_obs_pts, my_planner.get_path());
    }
#endif

    return 0;
}
