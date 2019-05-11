#include "main.hpp"
#include "convexhull.hpp"

int main(int argc, char ** argv)
{

  std::ofstream myfile;
  std::ofstream barfile;
  myfile.open("input.dat");
 
  assert(argc==3);
    
  int noOfThreads = std::stoi(argv[1]);
  int noOfPoins = std::stoi(argv[2]);
  
  std::vector<Point> points(noOfPoins);
  std::cout<<"\n\n\nGenerating points........\n\n\n"<<std::endl;

  for (int i = 0; i < noOfPoins; i++)
  {
    points[i].x = rand()%100;
    points[i].y = rand()%125;
    myfile<<points[i].x<<" "<<points[i].y<<"\n";
  }
  
  myfile.close();
  
  /*
  * Sequentail funcation call and time analysis
  */ 

  auto start = std::chrono::high_resolution_clock::now();
  std::stack<Point> S = convexHull(points, noOfPoins);
  auto stop = std::chrono::high_resolution_clock::now();
  auto dt = stop - start;
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dt);
     
  /*
  * Parallel funcation call and time analysis
  */ 	    

  auto start_1 = std::chrono::high_resolution_clock::now();
  std::stack<Point> S1 = convexHull_1(points, noOfPoins,noOfThreads);
  auto stop_1 = std::chrono::high_resolution_clock::now();
  auto dt_1 = stop_1 - start_1;
  auto ms_1 = std::chrono::duration_cast<std::chrono::milliseconds>(dt_1);
  
  std::cout << "============================Report==============================="<<std::endl;
  std::cout<<"\n\nNumber of Points:\t"<<noOfPoins<<std::endl;
  std::cout << "\nSequentail Hull Took:\t" << ms.count() << " milliseconds"<< std::endl;
  std::cout << "\nParrallel Hull building Took ( " <<noOfThreads<<" ) :\t"<< ms_1.count() << " milliseconds\n"<< std::endl;
  std::cout << "================================================================="<<std::endl;
  
  if(noOfPoins <= 1000)
  {
 	 printOutPut(S,noOfPoins); 	
  }
 
   
    return 0;
}
