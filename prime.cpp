#include<prime.h>
#include<iostream>
#include<queue>
#include<semaphore.h>
#include<stdlib.h>
#include<mutex>
#include<condition_variable>

using namespace std;

sem_t sem;
sem_t pause;
condition_variable cv;
mutex mtx;
int count = 0;
bool qEmpty() {return count != 0;}

template <class T>
class safeQueue
{
  
private:
  queue<int> q;
  
public:
  safeQueue()
    {
      //Nothing in her  
    }
  //bool qEmpty() {return !q.empty();}  

  void Push(int i)
    {
      unique_lock<mutex> lock(mtx);
      sem_wait(&sem);
      q.push(i);
      count = q.size();
      sem_post(&sem);
      cv.notify_one();
    }
  
  int Pop()
    {
      
      unique_lock<mutex> lock(mtx);
      
      cv.wait(lock, qEmpty);//if queue is not empty
      
      if(!q.empty())
      	{
	  sem_wait(&sem);
	  count = q.size();
	  int popped = q.front();
	  q.pop();
	  sem_post(&sem);
	  return popped;
	}	
    }
  
  int Front()
    {
      unique_lock<mutex> lock(mtx);
      cv.wait(lock, qEmpty);
      if(!q.empty())
      {
	sem_wait(&sem);
	  int front = q.front();
	  sem_post(&sem);
	  return front;
      }	
    }
  
};


int primeFound=0;
int nums = 10;
void *threads(void* param)
{
  safeQueue<int> *oldsq = ((safeQueue<int>*)param); 
  safeQueue<int> newsq;
  int nPrime = oldsq->Front();
  bool newThreadCreated = false;
  cout<<"THREAD NUMBER ------ " << nPrime<<endl;
  while(true)
    {
      if(oldsq->Front()%nPrime != 0)///NUMBER Wasnt divisible by front element, therefore push it onto new queue, and generate new thread
	{
	  int poppedNum = oldsq->Pop();
	  newsq.Push(poppedNum);//this puts first element into new q, then we pass it into new thread;
	  if(newThreadCreated == false)
	   {
	      primeFound = primeFound+1;
	      if(primeFound == nums)
		{
		  cout<<"WE FOUND THEM ALL"<<endl;
		  exit(0);
		}
	      pthread_t *tid  = new pthread_t();
	      pthread_create(tid,NULL,threads,&newsq);	    
	      newThreadCreated = true;
	   }
	}
      else// if(oldsq.Front()%nPrime == 0)
	{
	  //This means element is divisible by some number so remove it from the queue
	  oldsq->Pop();
	}
    }
  
}


void mainThread(int n)
{
  pthread_t tid;
  safeQueue<int> sq;
  bool found = true;
  int i = 2;
  while(found)
    {
      //Generate 2-n
      sq.Push(i);
      if(i == 2)
	{
	  pthread_create(&tid,NULL,threads,&sq);
        }
      if(primeFound == n)
	{
	  //found = false;
	  exit(0);
	}
      i++;
     }
  
}

int main(void)
{
  sem_init(&pause, 0, 0);
  sem_init(&sem, 0, 1);
  cout<<"Enter amount of primes you wish to find: "<<endl;
  cin>>nums;
  mainThread(nums);
  return 1;
}

