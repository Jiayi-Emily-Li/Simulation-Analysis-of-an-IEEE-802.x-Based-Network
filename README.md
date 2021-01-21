# Simulation-Analysis-of-an-IEEE-802.x-Based-Network

This project is about to implement a discrete event simulation model. In this model, we need to
implement the link processor, which is the server, and the buffer, which we use queue to
implement it. The important part for this model is to create arrival and departure time for each
packet based on distribution and trigger different actions according to the state of the queue and
server. After that, we can calculate the average queue-length and the mean server utilization. The
average queue-length is the average number of waiting packets in buffer. The mean server
utilization is the average percent of busy time of the server.
