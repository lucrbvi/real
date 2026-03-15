include Real

a = 0

puts Window.ready?

Window.open do
 a = a + 1
 puts a

 if a == 120
   puts Window.ready?
   Window.close
 end
end
