2/5/17

author: John Nelson

This is the source code for my 3D mobile graphics project.

Program is written in OpenGL with freeglut and GLEW. Your computer must have OpenGL v4.3 at least.
It uses the freeglut 3.0.0 library: http://freeglut.sourceforge.net/
It also uses glew 2.0

The program reads in 7 model files from ply_files. These files are turned into mesh objects called MyMesh. 
GenerateGeometry passes the point, color, and normal data for each model mesh, mobile arm, and axis line to the array object. 
Each mobile arm is its own mesh object in my code. Also, every model is a child of an empty mesh object in the model hierarchy. 
I did this so that I could rotate the teapot and galleon to be right side up without messing up the rotation of the rest of the mobile. 
My method of the matrix stack was to recursively pass through the parents of a mesh object when computing the CTM. 
The individual objects rotate faster than the arms to make the rotations noticeable.

Commands:
e - draws bounding boxes around models
s - mobile arms will shrink and stretch in a sinusoid motion
p - increases spotlight cutoff angle
P - decreases spotlight cutoff angle
m - scene uses per-pixel shading
M - scene uses vertex/flat shading

My hierarchy structure is as follows:

Empty root
->Cow Model
->Level 1 Arm
- ->Empty Node
- - ->Ant Model
- ->Empty Node
- - ->Teapot Model
- - ->Level 2 Arm
- - - ->Empty Node
- - - - ->Shark Model
- - - ->Empty Node
- - - - ->Beethoven Model
- - - - ->Level 3 Node
- - - - - ->Empty Node
- - - - - - ->Big_dodge Model
- - - - - ->Empty Node
- - - - - - ->Galleon Model

The program runs at 30 Frames Per Second.
Note: The cow is white, so it disappears into the background when it is in the area of the spotlight. My spotlight simply maximizes the color value of a vertex.
