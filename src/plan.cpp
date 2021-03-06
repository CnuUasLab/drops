///////////////////////////////////////////////////////////////////////////////
// plan.cpp - Planning for DROPS - Dynamics Realtime Obstacle Pathing System
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

#include "plan.hpp"

Planner::Planner(): planning_time(10.0),
    initial_epsilon(3.0),
    search_forward(false),
    changed(false)
{

}

Planner::~Planner()
{
    delete m_planner;
    m_planner = NULL;
}

/*
 * Actually do the planning
 * Returns 0 if path exists, else Planner::PATH_EXISTS
 */
int Planner::plan()
{

    if(changed) {

        if(dynamic_cast<ADPlanner*>(m_planner) != NULL) {
            //Get changed states and update them.
            if(search_forward) {
                std::vector<int> succs_of_changed;
                m_env.GetSuccsofChangedEdges(&changed_cells, &succs_of_changed);
                ((ADPlanner*)m_planner)->update_succs_of_changededges(&succs_of_changed);
            } else {
                std::vector<int> preds_of_changed;
                m_env.GetPredsofChangedEdges(&changed_cells, &preds_of_changed);
                ((ADPlanner*)m_planner)->update_preds_of_changededges(&preds_of_changed);
            }
        } else if (dynamic_cast<ARAPlanner*> (m_planner) != NULL) {
            ((ARAPlanner*)m_planner)->costs_changed(); //use by ARA* planner (non-incremental)
        }

        changed_cells.clear();
        changed = false;

    }

    std::vector<int> solution_IDs;
    bool path_exists = (m_planner->replan(planning_time, &solution_IDs) == 1);

    //Maybe print out something here about the path
    xythetaPath.clear();
    m_env.ConvertStateIDPathintoXYThetaPath(&solution_IDs, &xythetaPath);

    last_plan_good = path_exists;

    if(path_exists) {
        return Planner::PATH_EXISTS;
    } else {
        return 0;
    }

}

/*
 * initialize the planner based on data give to us
 * returns 0 on success, otherwise some error code
 */
int Planner::initialize(env_data_t &env_data, env_constants_t &env_const)
{
    if(m_planner == NULL) {
        if(init_planner() != 0) {
            return 1;
        }
    }
    changed = true;
    bool ret = m_env.InitializeEnv(env_data.width, env_data.height, env_data.grid_2d,
                                   env_data.start_x, env_data.start_y, DEG_TO_RAD(env_data.start_theta % 360),
                                   env_data.end_x, env_data.end_y, DEG_TO_RAD(env_data.end_theta % 360),
                                   0.0, 0.0, 0.0, //These params are unused
                                   perimeterptsV, env_const.cellsize_m,
                                   env_const.est_velocity, env_const.timetoturn45degs,
                                   env_const.obs_thresh, env_const.motion_prim_file);
    if(!ret) {
        //Failed to initialize env
        return 1;
    }
    if(!m_env.InitializeMDPCfg(&MDPCfg)) {
        return 2;
    }
    if (m_planner->set_start(MDPCfg.startstateid) == 0) {
        return 3;
    }
    if (m_planner->set_goal(MDPCfg.goalstateid) == 0) {
        return 4;
    }
    return 0;
}

/*
 * Updates the dynamic points of the graph. Used for moving obstacles
 */
int Planner::update_grid_points(point_char_map &points)
{
    nav2dcell_t nav2dcell;
    for(auto it = points.begin(); it != points.end(); it++) {
        m_env.UpdateCost(it->first.first, it->first.second, it->second);
        nav2dcell.x = it->first.first;
        nav2dcell.y = it->first.second;
        changed_cells.push_back(nav2dcell);
    }
    changed = true;

    return 0;
}

/*
 * initialize the palnner. Should be called only once during construction
 */
int Planner::init_planner()
{
    m_planner = new ADPlanner(&m_env, search_forward);

    m_planner->set_initialsolution_eps(initial_epsilon);
    m_planner->set_search_mode(false); // Search beyond first solution

    return 0;
}

/*
 * used to set the start and end state for the planner.
 */
int Planner::set_planner_states(int start_state_id, int goal_state_id)
{
    if(m_planner == NULL) {
        return 1;
    }
    if(m_planner->set_start(start_state_id) == 0) {
        return 2;
    }
    if(m_planner->set_goal(goal_state_id) == 0) {
        return 3;
    }
    return 0;
}

/*
 * returns the path vector if good, otherwise an empty vector
 */
std::vector<sbpl_xy_theta_pt_t> Planner::get_path()
{
    if(last_plan_good) {
        return xythetaPath;
    } else {
        return std::vector<sbpl_xy_theta_pt_t>();
    }
}
