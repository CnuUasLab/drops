#Test Cases
These are the test cases to be used for testing the DROPS client along with the _JAM Interoperability server.

One Stationary Obstacle
-----------------------
<b>Description: </b>One stationary obstacle in the search area grid.</br>
<b>Expected: </b>That the plane will avoid the obstacle and the goal</br>
                  will be on the path that the plane was origionally going to travel. </br>
                  _JAM's HTTP server should return a JSON obsject with an element in </br>
                  the stationary array under obstacles.</br>


Two Stationary Obstacles
------------------------
<b>Description: </b>One stationary two obstacles in the search area grid, which will be in close proximity.</br>
<b>Expected: </b> The DROPS client should come up with a path that avoids both obstacles and </br>
                    the _JAM HTTP server would return a JSON object that contains both obstacles in the stationary</br>
                    obstacle array.</br>


One Moving Obstacle
-------------------
<b>Description: </b>One moving obstacle in the search area grid.</br>
<b>Expected: </b> New path should avoid th obstacle and the JSON object returned from the _JAM HTTP</br>
                  server should contain the obstacle in the moving obstacle array.</br>

Two Moving Obstacles
--------------------
<b>Description: </b>Two moving obstacles in the search area grid, which will be in close proximity.</br>
<b>Expected: </b> New path should avoid both moving obstacles and the JSON object returned from the _JAM HTTP</br>
                  server should contain both obstacle in the moving obstacle array.</br>
                  
Stationary Obstacle over Defined Waypoint
-----------------------------------------
<b>Description: </b>The scenario exists that we may have an obstacle that is presented over a needed waypoint which</br>
                    would interfere with the Search Area Task.</br>
<b>Expected: </b> The plane should avoid the obstacle and should be able to cover the search area at the same</br>
                    time as avoiding the obstacle. The JSON object returned by the _JAM HTTP server should present</br>
                    all obstacles within the search area grid.</br>


