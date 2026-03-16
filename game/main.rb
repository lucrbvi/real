include Real

puts Window.ready?
a = 400

Window.size = Vector2.new(100, 100)

puts sleep(2)

# main game loop
Window.open do
  if a <= 0
    a = 800
  end

  puts Window.position

  Window.title = "Real | #{Window.resolution}"
  a -= 1

  Window.size = Vector2.new(a, a)
end

