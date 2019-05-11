#ifndef _convexhull_
#define _convexhull_

#include <fstream>

/*
* Function Distance calculates the distance between points and sends the result
*/

int distanceBpoints(Point a, Point b)
{
    auto val = pow((a.x - b.x),2) + pow((a.y - b.y),2);
    return val;
}

/*
* Function checkOrientation checks for the left or right turn
*/ 
int checkOrientation(Point p, Point q, Point r)
{
    auto val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);
 
    if (val == 0) { return 0; }  
    else
    {
      if(val > 0) { return 1; }
      else { return 2; }
    } 
}

/*
* Function maintainStack keeps the records of the points
*/

Point maintainStack(std::stack<Point> &Stack)
{
    Point temp = Stack.top();
    Stack.pop();
    Point point = Stack.top();
    Stack.push(temp);
    return point;
}

/*
* Function check validates if the two angles are same then select with the largest distance
*/
int check(const void *a, const void *b)
{
   Point *p1 = (Point *)a;
   Point *p2 = (Point *)b;
 
   // Find orientation
   auto val = checkOrientation(p0, *p1, *p2);
   if (val == 0)
   {
     if(distanceBpoints(p0, *p2) >= distanceBpoints(p0, *p1))  { return -1; }
     else  { return 1; }
   }
  else if(val == 2)
  {
    return -1;
  }
  return 1;
}


std::stack<Point> convexHull(std::vector<Point> points, int inputSize)
{
   int ymin = points[0].y, min = 0;
   for (int i = 1; i < inputSize; i++)
   {
     int y = points[i].y;
 
     if ((y < ymin) || (ymin == y &&
         points[i].x < points[min].x))
        ymin = points[i].y, min = i;
   }
  for (int w = 1; w < inputSize; w++)
  {
     int a = points[w].x;
     if ((a < ymin) || (ymin == a &&
     points[w].x < points[min].x))
     {
        a = ymin;
      }
  }
 
   
   Point temp = points[0];
   points[0] = points[min];
   points[min] = temp;

   p0 = points[0];
   qsort(&points[1], inputSize-1, sizeof(Point), check);
  
  int m = 1; 
     
  for (int i=1; i<inputSize; i++)
  {
    while (i < inputSize-1 && checkOrientation(p0, points[i],points[i+1]) == 0)
    i++;

    points[m] = points[i];
    m++;  
  }

   if (m < 3) { exit(1); }
   else{
   {
   	std::stack<Point> Stack;
   	Stack.push(points[0]);
   	Stack.push(points[1]);
   	Stack.push(points[2]);
   }
   

  for (int i = 3; i < m; i++)
  {
    while (checkOrientation(maintainStack(Stack), Stack.top(), points[i]) != 2) 
   	{ 
   		Stack.pop(); 
   	}
    Stack.push(points[i]);
  }
  return Stack;
}

/*
* Parallel Version
*/


std::stack<Point> convexHull_1(std::vector<Point> points, int inputSize, int noOfThreads)
{
   int ymin = points[0].y, min = 0;
 omp_set_num_threads(noOfThreads);
  
   #pragma omp parallel num_threads(omp_get_max_threads())
    {
      #pragma omp for
   for (int i = 1; i < inputSize; i++)
   {
     int y = points[i].y;
 
     if ((y < ymin) || (ymin == y &&
         points[i].x < points[min].x))
        ymin = points[i].y, min = i;
   }
 }
   
   Point temp = points[0];
   points[0] = points[min];
   points[min] = temp;

   p0 = points[0];
   qsort(&points[1], inputSize-1, sizeof(Point), check);
  
  int m = 1; 

/*
* Parallelize the loop for selecting the values in the intial stack
* num_threads are defined by the user 
* Schedule guided: Divides the loop in chuck and each thread then performs chunk of operations and then request another chunk
* Chunk size is proportional to the number of unassigned iterations divided by the number of the threads
*/  
  #pragma for
  for (int i=1; i<inputSize; i++)
  {
    #pragma while
    while (i < inputSize-1 && checkOrientation(p0, points[i],points[i+1]) == 0)
    i++;
         
    points[m] = points[i];
    m++;  
  }
    
  if (m < 3) { exit(2); }
  else
  {
  	std::stack<Point> Stack;
  	Stack.push(points[0]);
  	Stack.push(points[1]);
  	Stack.push(points[2]);	
  }
  

  for (int i = 3; i < m; i++)
  {
    while (checkOrientation(maintainStack(Stack), Stack.top(), points[i]) != 2) 
   	{ 
   		Stack.pop(); 
   	}
    Stack.push(points[i]);
  }

  return Stack;
} 

/*
* Print the output if the input size less then 1000
*/

void printOutPut(std::stack<Point> &Stack,int size)
{
  std::ofstream myfile;
  myfile.open("output.dat");

    int flag = 1;
    int a,b;

    while (!Stack.empty())
    {
      Point p = Stack.top();
      myfile<<p.x<<" "<<p.y<<"\n";
        if(flag)
        {
          flag =0;
          a = p.x;
          b = p.y;
        }
          Stack.pop();
    }
      myfile<<a<<" "<<b<<"\n";
      myfile.close();  
    
      auto r = system("gnuplot -p -e \"plot 'output.dat' using 1:2 title 'convexHull' with lines, 'input.dat' using 1:2 title 'input' with points \"");

}

#endif
