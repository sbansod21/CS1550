public class Node<T> {
  public int dirtyBit = 0;
  public T data;
  public Node<T> next;

  public Node() {
    this(null, null);
  }

  public Node(T data) {
    this(data, null);
  }

  public Node(T data, Node<T> next) {
    setData(data);
    setNext(next);
  }

  public T getData() {
    return data;
  }

  public Node<T> getNext() {
    return next;
  }

  public void setData(T data) {
    this.data = data;
  }

  public void setNext(Node<T> next) {
    this.next = next;
  }

  public String toString() {
    return "" + getData();
  }

  public int getDirtyBit() {
    return dirtyBit;
  }

  public void setDirtyBit() {
    dirtyBit = 1;
  }
} // EOF NODE
