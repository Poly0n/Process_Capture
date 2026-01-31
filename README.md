#  Windows Procces Capture

As i'm sure all us Windows Users know, Microsoft doesn't care about performance or ease of use these days.
So I decided to take make a alternative that strives to give users insight into their computer resources
and running processes without the slow and janky task manager. This app is a work-in-progress.

This program captures your computer's activity like a snapshot. To get full use out of it, please use the following commands:

##  Commands
- help           *Displays Commands*
- pid (number)   *Retrieves Program Name Associated With The Process ID.*
- kill (pid)     *Terminates Specified Process*
- find (name.exe)  *Finds Processes PID*
- filter (name.exe)          *Filters Processes To Only Display Processes Specified By The User.*
- path (pid)      *Shows The Full File Path Of A Running Process.*
- refresh         *Refreshes The Screen To Update Results. Helps resetting your filter*
- q  *Quits Application*

##  Preview

<img width="1021" height="783" alt="image" src="https://github.com/user-attachments/assets/cca349d1-ce1c-4487-a5f0-c551a5000a00" />


### Prerequisites
- C++20 compatible compiler (GCC, Clang, MSVC)

### Installation

```bash
# Clone the repository
git clone https://github.com/Poly0n/Process_Capture
cd Process_Capture

# Build the project
g++ -std=c++20 -o ProcessMonitor Process_Monitor_Project.cpp Processes.cpp Execution.cpp -lpdh -lpsapi
