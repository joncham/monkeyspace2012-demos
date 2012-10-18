#!/bin/sh -x
gmcs Core.cs -t:library
gmcs Add.cs -r:Core.dll -t:library
gmcs Subtract.cs -r:Core.dll -t:library
gmcs Multiply.cs -r:Core.dll -t:library
gmcs Divide.cs -r:Core.dll -t:library
