#include <iostream>
#include <list>
#include <vector>
#include <random>
#include "packet.h"
#include "destination.h"
#include "event.h"
#include "transmission.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

/* A faire
 * - traiter les evenements 1 a 1 --> transmission (schedule next Transmission) arrival (schedule nex arrival)
 */ 

using namespace std;

//Global variables 
double currentTime=0.0;
double startTxTime=0.0, stopTxTime=0.0; 

list<event> events;					//The events (discrete event simulator) - it is not a queue because it is not excatly FIFO (we dequeue at the fornt but insertion of event is not neceessarily at the back/end)
vector<destination> destinations;	//The destinations
list<packet> buffer;				//The buffer

mt19937 engine;						//The random number generator (the discrete one)

//Functions


// double getSampleExpDistribution(double lambda)//lambda is 1/E[X]   
// {
//   std::exponential_distribution<> randomFlot(lambda);
//   return(randomFlot(engine));
// }

/**
 * @brief Get the Sample Poisson Distribution object, lambda is the mean of the Poisson distribution
 * 
 * @param lambda 
 * @return int 
 */

int getSampleExpDistribution(double lambda) // lambda is the mean of the Poisson distribution
{
    std::poisson_distribution<> randomInt(lambda);
    return randomInt(engine);
    spdlog::trace("Poisson distribution generator");
}


// print an event's attribute
/**
 * @brief Get output stream, print data of each event instance
 * 
 */
void printEvents()
{
  std::list<event>::iterator it;
  spdlog::trace("Print all events of list");
  for(it=events.begin(); it!=events.end(); it++) { // get output stream, print data of each event instance
          if (it->m_type==0) spdlog::debug("Event: Arrival / dest= {} / time= {}", it->m_dest.m_no, it->m_time);
          else spdlog::debug("Event: Transmission / dest= {} / time= {}", it->m_dest.m_no, it->m_time);
  }
}


/**
 * @brief Insert given event object to the events list, then sort the list with time ordered
 * 
 * @param anEvent 
 */
void insertEvent(class event anEvent)
{
  std::list<event>::iterator it;

  it=events.begin();

  // sort the event
  while(anEvent.m_time > it->m_time && it!=events.end()) it++;  
  spdlog::trace("Insert event and sorted by time");
  if (anEvent.m_type==0) spdlog::debug("Event: Arrival / dest= {} / time= {}", anEvent.m_dest.m_no, anEvent.m_time);
  else spdlog::debug("Event: Transmission / dest= {} / time= {}", anEvent.m_dest.m_no, anEvent.m_time);

  events.insert(it,anEvent);      // insert an new event element to the list, with corresponding iterator
  
}

//Add the next arrival to the list of events for a given destination


// For 1 packet
/**
 * @brief Add arrival event with given destination for scheduling, time between packets transmitted correspond to each type of distribution is calculated here
 * 
 * @param dest 
 */
void addNextArrival(class destination dest)  // given destination
{
	double timeCalculate;

  switch(dest.m_arrivalDistribution)    // Deterministic or Poisson
  {
    case 0: timeCalculate=currentTime+1.0/dest.m_arrivalRate; break;      // case 0 is deterministic, time = current + 1/ numbers of packet => time per packet
    case 1:                                                               // Poisson distribution
    {//Les accolades sont necessaires car elle fixe la portee de la declaration de la variable ci-dessous
	    double value=getSampleExpDistribution(dest.m_arrivalRate)/(dest.m_arrivalRate*13);    // time based of arrival rate
	    timeCalculate=currentTime+value;      // time = current time + arrival time
      spdlog::debug("rand exp value: {}", value);
	    break;
    }
    default: cerr << "[ERROR] in addNextArrival(): the destination m_arrivalDistribution member is incorrect (=" << dest.m_arrivalDistribution << ")" << endl;
	     exit(1);
  }
 
  //Insert this event in the list
  event newArrival(0, dest, timeCalculate);  // create new event that arrival, given destination, time for delivery 1 packet
  spdlog::trace("Add new arrival event");
  insertEvent(newArrival);                   // insert into event list (sorted)
}




//Schedule the next transmission 
/**
 * @brief Scheduling transmission with given time
 * 
 * @param time 
 */
void scheduleNextTransmission(double time)    // Function for plan the next transmission (given time that transmission occured)
{
  event newTrans(1,-1,time);	  // transmission event (1), no destination => -1, given time
  spdlog::trace("Add new transmission event");
  insertEvent(newTrans);      // insert event to the list (both sort)
}

void logger(string level_log)
{
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_color(spdlog::level::trace, console_sink->yellow); 
  auto logger = std::make_shared<spdlog::logger>("logger", console_sink);
  spdlog::set_default_logger(logger);
  if (level_log == "info") {
    spdlog::set_level(spdlog::level::info);
  } 
  else if (level_log == "debug") {
    spdlog::set_level(spdlog::level::debug);
  } 
  else if (level_log == "trace") {
    spdlog::set_level(spdlog::level::trace);
  } 
  else {
    spdlog::set_level(spdlog::level::info);  // Default to info if no match
  }
}

// Notable function: STARTING SIMULATION
/**
 * @brief STARTING SIMULATION 
 * 
 * @param buffer List of packets
 * @param discipline Discipline: 0: FIFO, 1: fifoOFDMAoptimal, 2:fifoAggregation
 */
void simulateQueue(list<packet> &buffer, int discipline)      // given packet list and discipline
{ 
  int nbOfTransmissions=0;
  int maxTransmission=1000;
  double timeToTransmit;
  
  
  spdlog::info("Start simulating...");
  spdlog::info("Max number of transmission = {}", maxTransmission);
  //Start of the simulation: we add an arrival for each destination
  if(events.empty())    // the begin, list is empty cause there are no event occur
  {   
    spdlog::info("begin of process, the event list is empty, adding arrival with all destination");
    for(auto it = std::begin(destinations); it != std::end(destinations); ++it)    // iterate all destination and add arrival to the event list based on time
    {
      spdlog::info("add new arrival");
      addNextArrival(*it);               // all arrival is added (starting simulation)
    } 
  }
  spdlog::debug("Event list before processed");
  printEvents();                    // Print all the event added (all sorted)


  while (nbOfTransmissions < maxTransmission)
{
    std::list<event>::iterator it;              // iterate each element on event list

    //What is the next event
    spdlog::debug("Queue component, current list of events");
    printEvents();

    it=events.begin();
    spdlog::info("Current time is taking value of the FIFO event in event list, before updated Current time = {}", currentTime);
    currentTime=it->m_time;     
    spdlog::info("After updated Current time = {}", currentTime);
    event thisEvent=*it;		//We copy this event as we remove it from the list      // Copy the current event
    events.pop_front();			//We remove this event as it is process	                // Remove the each first event

    if (thisEvent.m_type==0) spdlog::info("Queue component, we are processing => Event: Arrival / dest= {} / time= {}", thisEvent.m_dest.m_no, thisEvent.m_time);
    else spdlog::info("Queue component, we are processing => Event: Transmission / dest= {} / time= {}", thisEvent.m_dest.m_no, thisEvent.m_time);
    
    printEvents();

  switch(thisEvent.m_type)  
    {
      //arrival : for scheduling
      case 0: 
      {                       // current time greater transmission time before // timestamp
        spdlog::trace("Handling arrival event");
        spdlog::debug("Current time = {}, StopTxTime = {}", currentTime, stopTxTime);
        if(buffer.empty() && (currentTime > stopTxTime)) {
          spdlog::trace("empty buffer and current time is greater than stop transmission time");
          scheduleNextTransmission(currentTime); }             // If sending all packet, and current time is greater than scheduled for each transmission time => schedule the next transmission is the current time
        else if(buffer.size()>0 && currentTime < stopTxTime && thisEvent.m_dest.m_arrivalDistribution == 0){      // if have packets in buffer when scheduling, send it immediately
              spdlog::trace("Cause of Deterministic, send the buffer immediatly");
              spdlog::info("Current time before update = {}", currentTime);
              currentTime = currentTime + transmitNextPackets(discipline, buffer, currentTime);           // Update current time    (This case only happens in Deterministic mode)
              spdlog::info("Updated current time = {}", currentTime);
              nbOfTransmissions++;
              spdlog::info("Number of transmission completed = {}", nbOfTransmissions);
        }
        else{ 
          spdlog::trace("empty buffer and current time is lesser than stop tranmission time");
          scheduleNextTransmission(stopTxTime);
        }

        //If the buffer is empty we transmits at the arrival of this packet, but we have to check for are we transmitting or not?
        spdlog::info("Queue component, an Arrival event comes. Before addNextArrival(), number of Packet in Buffer = {}", buffer.size());        

        if (thisEvent.m_dest.m_arrivalDistribution == 0) {spdlog::trace("How set up the next arrival => case 0: timeCalculate=currentTime+1.0/dest.m_arrivalRate");}
        else {spdlog::trace("set up the next arrival => case 1:  double value=getSampleExpDistribution(dest.m_arrivalRate)/(dest.m_arrivalRate*13)");}
        addNextArrival(thisEvent.m_dest);//schedule the next arrival for this destination => cause infinitty
          class packet newPacket(3000,currentTime,thisEvent.m_dest);//1000 bytes (needs to be changed) // create new packet
          // class packet newPacket1(2000, currentTime,1);
          spdlog::info("new packet size = {}, destination = {}, time arrival = {}",newPacket.m_size,newPacket.m_destination.m_no,newPacket.m_arrival);
          // spdlog::info("new packet size = {}, destination = {}, time arrival = {}",newPacket1.m_size,newPacket1.m_destination.m_no,newPacket1.m_arrival);
          // add buffer for the transmission is scheduled
          buffer.push_back(newPacket);                              // add to end of the packets list (if arrival)
          // buffer.push_back(newPacket1);                              // add to end of the packets list (if arrival)

        spdlog::info("Queue component, Current number of Packet in Buffer = {}", buffer.size());        
        break;
      }
      //transmission: sending
      case 1: 
          spdlog::trace("Handling transmission event");
          if(buffer.empty()){spdlog::error("buffer is empty"); break;}//If the buffer is empty there is no transmission (no packet for sending)
          if(currentTime < stopTxTime) { spdlog::error("[ERROR] in another transmission");scheduleNextTransmission(stopTxTime); break; }  //We have to defer this transmission because we are being in another transmission  
            else {
                  spdlog::info("Queue component, buffer size = {}, start tx time = {}",buffer.size(), currentTime);
                  startTxTime = currentTime;  
                  timeToTransmit = currentTime + transmitNextPackets(discipline, buffer, currentTime);
                  // scheduleNextTransmission(timeToTransmit);
                  nbOfTransmissions++;                          // transmission done
                  stopTxTime = timeToTransmit;                  // time to done a transmisstion
                  currentTime = stopTxTime;
                  spdlog::info("Queue component, a packet has been transmitted, buffer size = {}, stop tx time = {}", buffer.size(), stopTxTime);
                  spdlog::info("Update current time to finishing transmission = {}", currentTime);
                  cout<<"[END OF TRANSMISSION]\n\n\n"<<endl;
                  break; }
      default: cerr << "[ERROR] in simulateQueue(): the event type is incorrect (=" << thisEvent.m_type <<")\n";
          exit(1);
    }

}

  printEvents();
}

int main(int argc, char* argv[])
{
  
  std::string level_log;
  if (argc > 1) {
        std::string arg = argv[1];
        if (arg.find("--log_level=") == 0) {
            level_log = arg.substr(12);
        } 
        else {
            spdlog::error("MISSING LOG ARGUMENT: Use --log_level=<level>");
            return -1;  
        }
    } else {
        spdlog::error("No arguments provided. Use --log_level=<level>");
        return -1;
    }

  logger(level_log);

  int nbOfDest=2;	
  /* initialize random seed: */
  srand (time(NULL));

  //Set destination 
  // class destination dest1(1,1,500.0,0), dest2(2,1,500.0,0), dest3(3,1,500.0,0); 

  class destination dest1(1,1,500.0,1), dest2(2,1,400.0,1), dest3(3,1,320.0,1), dest4(4,1,600.0,1), dest5(5,1,275.0,1), dest6(6,1,330.0,1);
  destinations.push_back(dest1);
  destinations.push_back(dest2);
  destinations.push_back(dest3);
  destinations.push_back(dest4);
  destinations.push_back(dest5);
  destinations.push_back(dest6);

   
  //class packet myPacket(1,2,3);
  //cout << myPacket.m_destination << endl;
  //buffer.push_back(myPacket);

  //The simulator
  
  simulateQueue(buffer, 1);

  return(0);
}
