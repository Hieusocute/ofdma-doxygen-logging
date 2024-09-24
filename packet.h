#ifndef PACKET_H
#define PACKET_H

#include "destination.h"

/**
 * @class packet
 * @brief Represents a network packet in the simulation.
 * 
 * This class models a packet, including its size, arrival time, 
 * and destination information.
 * 
 * ##### Attributes:
 * 
 * - int m_size:  
 *   The size of the packet in bytes.
 * 
 * - double m_arrival:  
 *   The time when the packet arrives.
 * 
 * - destination m_destination:  
 *   The destination to which the packet is sent.
 */
class packet {
	public:
		int			m_size; 		   //size in bytes
		double 		m_arrival; 		   //arrival time 
		destination m_destination; //integer that identifies the destination
	
  		packet(int size, double arrival, destination dest);
		bool operator==(packet &aPacket);
		~packet();
}; 

#endif
