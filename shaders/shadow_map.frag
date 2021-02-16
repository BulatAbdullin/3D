#version 330 core

void main()
{             
    /* do no processing */
    gl_FragDepth = gl_FragCoord.z; // update depth buffer
} 
