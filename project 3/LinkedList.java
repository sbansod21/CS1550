public class LinkedList<T> 
  {
  
  public Node<T> head;
  public Node<T> tail;
  public int size;

  public LinkedList() {
    head = null;
    tail = null;
    size = 0;
  }

  public T getHead() {
    return head.getData();
  }

  public void setDirty(final T data, String atype)
  {
    Node<T> curr = search(data);
    if(atype.equals("s"))
    {
      curr.setDirtyBit();
    }
  }

  public boolean getDirty(final T data) {
    Node<T> curr = search(data);

    if (curr.getDirtyBit() == 1) {
      return true;
    }

    return false;
  }

  public String toString() {
    String toString = "[ ";

    for (Node<T> curr = head; curr != null; curr = curr.getNext()) {
      toString += curr.getData();
      if (curr.getNext() != null) toString += ", ";
    }
    toString += " ]";

    return toString;
  }

  public void insertAtTail(final T data) {
    Node<T> n = (new Node<T>(data));
    size++;
		if (head == null) // if theres no element after head then..
		{
			head = n;
      tail = n;
		} else {      
      tail.next = n;
      tail = n;
		}
  }

  public void removeHead()
  {
    if (head == null){
      return;
    }
    if(this.size == 1)
    {
      head = null;
      tail = null;
    }else{
          head = head.next;
    }
    size--;
  }

  public int size() {
  

		return this.size;
  }

  public boolean contains(final T key) {
    return (search(key) != null);
  }

  public Node<T> search(final T key) {
    for(Node<T> curr = head; curr != null; curr = curr.getNext())
		{
			if(key.equals(curr.getData()))
			{
				return curr;
			}
			
		}
			return null;
  }
  public boolean empty()
	{
		return this.size() == 0;
	}

  public boolean remove(T key) {
     if(this.empty())
		{
			return false;
		
		}else if (head.getData().equals(key))
		{
			head = head.getNext();
      size--;
			return true;

		}else
		{
			Node<T> curr = head;

			while(curr.next != null)
			{
				if(curr.next.data.equals(key))
				{
          if(curr.next == tail)
          {
            tail = curr;
            size--;
            return true;
          }else{
            curr.setNext(curr.next.next);
            size--;
					  return true;
          }
					
				}
				
				curr = curr.getNext();
			}

			return false;
  }
  }
} // EOF LINKED LIST