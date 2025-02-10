#ifndef DESTINATION_H
#define DESTINATION_H

/**
 * @class destination
 * @brief Represents a network destination for packet transmission.
 * 
 * This class handles the configuration and management of a destination, 
 * including address, modulation scheme, and packet arrival details.
 * 
 * ##### Attributes:
 * - int m_no:  
 *   Destination address (1, 2, etc.).
 * 
 * - int m_mcs:  
 *   Modulation and Coding Scheme (MCS) index.
 * 
 * - double m_phyRate:  
 *   Physical Transmission Rate in Megabits per second (Mbit/s).
 * 
 * - double m_arrivalRate:  
 *   Number of packets transferred per second for this source.
 * 
 * - int m_arrivalDistribution:  
 *   Distribution type for interarrival times:
 *   - Deterministic (0): time between events is constant.
 *   - Poisson (1): random events that occur independently with a constant average rate.
 */
class destination {
	public:
		int		m_no;					//destination address: 1, 2, etc.
		int		m_mcs;					//MCS index
		double	m_phyRate;				//Physical Trans Rate (Mbit/s) 
		double	m_arrivalRate;			//Number of packet per second for this source
		int		m_arrivalDistribution;	//Type of distribution for the interarrival: Deterministic (0) time between events is constant,
										// 											 Poisson(1), etc.: random events that occur independently and with a constant average rate. 

//phyRate will be set according to the mcs in the constructor (in the .cc file)
destination (int no=0, int mcs=0, double arrivalRate=1.0, int arrivalType=0);  		 
//MCS is not specified here (we can put whatever we want for the phyRate) //A recoder pour prendre en compte les autres...
destination (double phyRate, int no, double arrivalRate, int arrivalType);
bool operator==(destination &aDest);
~destination ();
};

#endif
