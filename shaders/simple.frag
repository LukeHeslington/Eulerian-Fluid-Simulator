#version 330 core
out vec4 finalColor;
in vec2 TexCoord;

uniform int objectType; // Add this line to get the objectType uniform
uniform vec3 currentColorForSquares;
uniform vec3 currentColorForCircle;

void main() {
    if (objectType == 0) {
        // Set the color to light sky blue for the circle
        finalColor = vec4(currentColorForCircle, 1.0);
    }
    else if (objectType == 1) {
        // Set the color to red for the squares
        finalColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else if (objectType == 2)
    {
        finalColor = vec4(currentColorForSquares, 1.0);
    }
}
