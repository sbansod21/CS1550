//imports
import java.io.*;
import java.lang.Math;
import java.util.*;


public class vmsim {
  //global vars
  public static String algo;
  public static int numFrames;
  public static int pageSize;
  public static String memSplit;
  public static File input;
  public static int p1;
  public static int p2;

  //for printing
  public static int memAccess = 0;
  public static int PageFaults = 0;
  public static int diskWrites = 0;

  public static void main(String[] args) {
    //handling the arguments
    if (args.length < 5 || args.length > 9) {
      System.out.println("Incorrect number of arguments!");
      System.exit(0);
    } else {
      //calling this method for cleaner code
      argsHandle(args);
    }

    //here is where im calling the algo
    if (algo.equals("opt")) {
      optimal();
      printStats();
    } else if (algo.equals("lru")) {
     LRU();
     printStats();
    } else {
      System.out.println("Incorrect algorithm input!");
      System.exit(0);
    }
  }

  public static void argsHandle(String[] args) {
    // ./vmsim -a <opt|lru> –n <numframes> -p <pagesize in KB> -s <memory split>
    // <tracefile>

    //name of algo
    if (!args[0].equals("-a")) {
      System.out.println(
        "Incorrect argument, '-a' expected, follwed by <opt||lru>."
      );
      System.exit(0);
    }
    algo = args[1];

    //number of frames
    if (!args[2].equals("-n")) {
      System.out.println(
        "Incorrect argument, '-n' expected, followed by the number of frames."
      );
      System.exit(0);
    }

    numFrames = Integer.parseInt(args[3]);

    //pagesize
    if (!args[4].equals("-p")) {
      System.out.println(
        "Incorrect argument, '-p' expected, followed by the pagesize in KB."
      );
      System.exit(0);
    }

    pageSize = Integer.parseInt(args[5]);

    //memory split
    if (!args[6].equals("-s")) {
      System.out.println(
        "Incorrect argument, '-s' expected, followed by the memory split."
      );
      System.exit(0);
    }
    memSplit = args[7];

    //math for the memory splt
    String strmem[] = memSplit.split(":");
    p1 = Integer.parseInt(strmem[0]);
    p2 = Integer.parseInt(strmem[1]);
    double x = (p1 * numFrames) / (p2 + p1);
    p1 = (int) x;
    p2 = numFrames - p1;

    //file name
    input = new File(args[8]);
  }

  public static int getAddy(String a) {
    //the adress is in hex so convert to binary
    a = a.substring(2);
    int addy = (int) Long.parseLong(a, 16);

    //its in KB so do the math
    int intoBytes = pageSize * (int) (Math.pow(2, 10));
    int x = (int) ((Math.log(intoBytes)) / (Math.log(2)));
    //shift right to get rid of the offset
    addy = addy >> x;

    return addy;
  }

  public static void LRU(){ 
    Scanner inScan;
    String atype;
    String address;
    int ptype;
    int frames;
    int ref;

    LinkedList<Integer> stack0 = new LinkedList<Integer>();
    LinkedList<Integer> stack1 = new LinkedList<Integer>();
    LinkedList<Integer> stack = new LinkedList<Integer>();

    //open the file
    try{
      inScan = new Scanner(input);
    }catch(FileNotFoundException fnf)
    {
      System.out.println("Error: File not found");
      System.exit(0);
      return;
    }
    //read the file
    while (inScan.hasNext()){
      memAccess++;
      atype = (String)inScan.next();
      address = inScan.next();
      ptype = inScan.nextInt();

      //change to the main one since there are 2 processes
      if(ptype == 0)
      {
        stack = stack0;
        frames = p1;
      }else{
        stack = stack1;
        frames = p2;
      }

      ref = getAddy(address); //do the address calculation
      
      if(stack.contains(ref)) //if the addy is already in our stack
      {
        if(stack.getDirty(ref)) //just move it to the top of the stack
        {
          stack.remove(ref);
          stack.insertAtTail(ref);
          stack.setDirty(ref,"s");//if the dbit was set then it remains set even if this is load
        }else{
          stack.remove(ref);
          stack.insertAtTail(ref);
          stack.setDirty(ref,atype);
        }
        
        //change out of the main
        if(ptype == 0)
        {
          stack0 = stack;
        }else{
          stack1 = stack;
        }
        continue; //we dont need to do anything else
      }

      //i think this is more efficent 
      //add first and then remove the LRU
      stack.insertAtTail(ref);
      stack.setDirty(ref,atype);

      //if after adding, theres more frames than allowed now evict
      if(stack.size > frames)
      {            
        PageFaults++;
        int lru = stack.head.data;//get the head of the list, I know this is the LRu
        if(stack.getDirty(lru) == true) //if its dirty then disl write
        {
          diskWrites++;
        }
        stack.remove(lru);//now we can remove
      }else{// this means that there are still spaces in the PTE
        PageFaults++;
      }

        //change back to the original
       if(ptype == 0)
        {
          stack0 = stack;
        }else{
          stack1 = stack;
        }
    }
    inScan.close();
  }

  public static void optimal(){
    //variables
    Scanner inScan;
    String atype;
    String address;
    int ptype;
    int frames;
    int ref;
    int lineNum = 1;

    //hashtables declaration (hashmap vs hashtable idk)
    HashMap<Integer,LinkedList<Integer>> table = new HashMap <Integer,LinkedList<Integer>>();
    HashMap<Integer,LinkedList<Integer>> table1 = new HashMap<Integer,LinkedList<Integer>>();
    HashMap<Integer,LinkedList<Integer>> table0 = new HashMap<Integer,LinkedList<Integer>>();
    
    //read the file 
    try{
      inScan = new Scanner(input);
    }catch(FileNotFoundException fnf)
    {
      System.out.println("Error: File not found");
      System.exit(0);
      return;
    }

    //now we add the page numbers to the hashtable
    LinkedList<Integer> x;
    while(inScan.hasNext())
    {
      inScan.next();
      address = inScan.next();
      ptype = inScan.nextInt();

      //switch into temp
      if(ptype == 0)
      {
        table = table0;
      }else{
        table = table1;
      }

      ref = getAddy(address); //address calculation

      LinkedList<Integer> y = new LinkedList<Integer>();
      if(table.containsKey(ref)) //if this addy is already in the table then just add a new page number to the linked lst
      {
        table.get(ref).insertAtTail(lineNum);  
      }else{ //esle add a new linked list and then add the page number
        table.put(ref, y);
        table.get(ref).insertAtTail(lineNum);  
      }
      lineNum++;

      //switch back
      if(ptype == 0)
      {
        table0 = table;
      }else{
        table1 = table;
      }

    }
    inScan.close();

    //re read the file
    try{
      inScan = new Scanner(input);
    }catch(FileNotFoundException fnf)
    {
      System.out.println("Error: File not found");
      System.exit(0);
      return;
    }

    //these are out PTE
    LinkedList<Integer> list = new LinkedList<Integer>();
    LinkedList<Integer> list0 = new LinkedList<Integer>();
    LinkedList<Integer> list1 = new LinkedList<Integer>();


    int line = 1;
    while(inScan.hasNext())
    {
      memAccess++;
      atype = (String)inScan.next();
      address = inScan.next();
      ptype = inScan.nextInt();

      //switch into it.
      if(ptype == 0)
      {
        list = list0;
        table = table0;
        frames = p1;
      }else{
        list = list1;
        table = table1;
        frames = p2;
      }
      ref = getAddy(address); //address calculation

      table.get(ref).removeHead(); //this removes the page number from the linked list
      
      if(list.contains(ref)) //if its already in then do the same thing as LRU
      {
        if(list.getDirty(ref)) //move to tail of ll
        {
          list.remove(ref);
          list.insertAtTail(ref);
          list.tail.setDirtyBit();
        }else{
          list.remove(ref);
          list.insertAtTail(ref);
          list.setDirty(ref,atype);
        }
        continue; //done
      }

      //unline LRU, here im doing the calculation first and then adding
      if(list.size == frames) //this means that the PTE is full and we need to evict
      {
        Node<Integer> curr = list.head;
        int max = -1;
        int add = 0;
        while(curr != null) //loop through our PTE
        {
          LinkedList<Integer> temp; 
          temp = table.get(curr.data); //this gets a linked list with the addy
          if(temp.size == 0) //if its empty then I know what I have to remove
          {
            add = (curr.data); // i know this is one I want to remove 
            break;
          }else{
            //this means that we are going to find a max
            
            int data = temp.head.data;
            if(data > max) //simple max loop, but im remembering which address I had to remove
            {
              max = data;
              add = curr.data;
            }
          }
          curr = curr.next; 
        }

          //at this point I know the thing I want to remove is in add
          if(list.getDirty(add) == true) //so do the diskwrites if needed
          {
            diskWrites++;
          }
          list.remove(add);
        
      }

      PageFaults++; //no matter what, this is a pagefault
      list.insertAtTail(ref);  
      list.setDirty(ref, atype);

      //switch back
      if(ptype == 0)
      {
        list0 = list;
        table0 = table;
      }else{
        list1 = list;
        table1 = table;
      }
    }
    
    inScan.close();
  }

  public static void printStats() {
    System.out.println("Algorithm: " + algo.toUpperCase());
    System.out.println("Number of frames: " + numFrames);
    System.out.println("Page size: " + pageSize + " KB");
    System.out.println("Total memory accesses: " + memAccess);
    System.out.println("Total page faults: " + PageFaults);
    System.out.println("Total writes to disk: " + diskWrites);
  }

} //EOF VMSIM



