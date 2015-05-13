#version 430 core
/**********************************************************************
** smepowercad
** Copyright (C) 2015 Smart Micro Engineering GmbH
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

layout ( triangles ) in;
layout ( triangle_strip, max_vertices = 3 ) out;
layout ( location = 48) uniform bool is_Selection;


in vec4 Color[];
in vec4 vTexCoord[];
in vec4 vVertexPosition[];

out vec4 gColor;
out vec4 gTexCoord;
out vec4 gVertexPosition;

void main()
{
    vec4 c[3];
    if (!is_Selection)
    {
        //calculate normal
        vec3 v0 = vec3(vVertexPosition[0].x, vVertexPosition[0].y, vVertexPosition[0].z);
        vec3 v1 = vec3(vVertexPosition[1].x, vVertexPosition[1].y, vVertexPosition[1].z);
        vec3 v2 = vec3(vVertexPosition[2].x, vVertexPosition[2].y, vVertexPosition[2].z);
        vec3 normal = normalize(cross(v1 - v0, v2 - v0));
        //calculate Color transformation
        float transformation = 0.3 * normal.z + 0.7;
        mat4 m = mat4(transformation, 0.0, 0.0, 0.0,
                      0.0, transformation, 0.0, 0.0,
                      0.0, 0.0, transformation, 0.0,
                      0.0, 0.0, 0.0, 1.0);

        c[0] = m * Color[0];
        c[1] = m * Color[1];
        c[2] = m * Color[2];
    }
    else
    {
        c[0] = Color[0];
        c[1] = Color[1];
        c[2] = Color[2];
    }

    gVertexPosition = vVertexPosition[0];
    gColor = c[0];
    gTexCoord = vTexCoord[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gVertexPosition = vVertexPosition[1];
    gColor = c[1];
    gTexCoord = vTexCoord[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gVertexPosition = vVertexPosition[2];
    gColor = c[2];
    gTexCoord = vTexCoord[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
