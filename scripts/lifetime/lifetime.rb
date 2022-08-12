class Counter
  def initialize(init)
    @count = init;
  end

  def count()
    prev = @count
    @count += 1
    prev
  end
end

def make_counter(init)
  count = init;
  Proc.new {
    prev = count
    count += + 1
    prev
  }
end

def exec_all()
  counter1 = make_counter(12)
  counter2 = make_counter(234)
  puts counter1.call()
  puts counter2.call()
  puts counter1.call()
  puts counter2.call()
end

exec_all()
