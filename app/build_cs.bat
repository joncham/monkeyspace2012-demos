csc -out:Core.dll -t:library Core.cs 
csc -out:Add.dll -r:Core.dll -t:library Add.cs 
csc -out:Subtract.dll -r:Core.dll  -t:library Subtract.cs 
csc -out:Multiply.dll -r:Core.dll  -t:library Multiply.cs 
csc -out:Divide.dll -r:Core.dll -t:library Divide.cs 
