<h1> Project 3 Writeup </h1>

In this project, we had to design a virtual memory simulator. I chose to use Java since its the langauge im most familiar,
along with the fact that there are many resources and built in libraries in Java which would make it easier for me to build
my simulator. When building any sort of a simulator, its important to test the ouput for efficiency and certain trends.
For now, I ran my LRU algorithm on different memory splits as well as pagesizes and number of frames. While doing this,
I used the provided 1.trace file. Here are the numbers:

![LRU Page faults table](https://github.com/cs1550-2214/cs1550-project3-sbansod21/blob/9618d9c8c8e82d0a041511798573deeca95d5478/images/LRU%20Page%20Faults%20table.JPG)

But the numbers are not great at illustrating the trends. Here is a graph:

![LRU Page Faults graph](https://github.com/cs1550-2214/cs1550-project3-sbansod21/blob/9618d9c8c8e82d0a041511798573deeca95d5478/images/LRU%20Page%20Faults%20Graph.JPG)

From this graph we can see that with lower pagesize and number of frames, there is big variablity in pagefaults with each memory split. 
The biggest being 9:7 and decreasing as the memory split decreases. When the Page size is increased to 4000KB or 4MB, there is no variabilty. 
The table shows us that with 16 frames, the numbers are close together with a range of about 100, this makes the graph look as if its the same
since the range is quite small. What is important to notice is that memory split 1:3 is an outlier is almost all the pagesize and frame number variations,
its possible that this outlier is negatively effecting the graph but I will overlook this in this analysis. At 4MB and 1024 frames, the number of
pagefaults are the same.

Now lets look at the Optimal algorithm. I ran this on the same file with the same memory split and page size and frame conbination.
Below are the numbers:

![OPT Paage Faults table](https://github.com/cs1550-2214/cs1550-project3-sbansod21/blob/9618d9c8c8e82d0a041511798573deeca95d5478/images/OPT%20Page%20Faults%20table.JPG)

As you can see the output is somewhat similar to that of the LRU. In fact, the number of pagefaults for 4MB and 1024 frames is the exact same.
This trend can be seen in the graph below.

![OPT Page Faults Graph](https://github.com/cs1550-2214/cs1550-project3-sbansod21/blob/9618d9c8c8e82d0a041511798573deeca95d5478/images/OPT%20Page%20Faults%20Graph.JPG)

This graph is also quite identical to the LRU one. There is a larger variability when the page size and frame number is smaller. 
