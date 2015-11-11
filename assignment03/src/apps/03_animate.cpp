#include "scene.h"
#include "image.h"
#include "tesselation.h"
#include "gls.h"

#include <cstdio>



void uiloop();

string scene_filename;          // scene filename
string image_filename;          // image filename
Scene* scene;                   // scene


// get keyframe interval that contains time 1)
pair<int,float> get_keyframe_details(const vector<int> &keytimes, int time) {
    auto interval = 0; //an index into the keytimes array at the interval of time
    auto t = 0.0f; //percent along in the interval (float between 0-1)
    
    // find interval of keytime where keytimes[interval] < time <= keytimes[interval+1]
    for (int idx = 0; idx < keytimes.size(); idx++){
        int minbound, maxbound; //represent the values at the interval boundaries
        minbound = keytimes[idx];
        maxbound = keytimes[idx + 1];
        // if time falls between min & max
        if ((time > minbound) && (time <= maxbound)) {
            // this value of idx is the interval index, no need to continue iterating
            interval = idx;
            break;
        }
    }
    // compute t
    t = (((float) time - keytimes[interval])/(keytimes[interval+1]-keytimes[interval]));

    //return interval and t
    return make_pair(interval,t);
}

// compute the frame from an animation *************HELP*****2) Ask about use of matrices
frame3f animate_compute_frame(FrameAnimation* animation, int time) {
    // find keyframe interval and t
    auto interval_t = get_keyframe_details(animation->keytimes, time);
    auto interval   = interval_t.first;
    auto t          = interval_t.second;
    vec3f rotatev1, transv1, rotatev2, transv2, transv, rotatev;
    mat4f rotatemx, rotatemy, rotatemz, transm, transform, framem, resultm;
    frame3f frame1, framenew;
    
    // get translation and rotation matrices by extracting them from both boundaries of the key frame
    rotatev1 = animation->rotation[interval];
    transv1 = animation->translation[interval];
    rotatev2 = animation->rotation[interval+1];
    transv2 = animation->translation[interval+1];

    //then weight them for this particular point in t
    rotatev = ((1-t)*rotatev1 + (t)*rotatev2);
    transv = ((1-t)*transv1 + (t)*transv2);
    
    //turn into matrices: rotates must be done separately for each direction
    rotatemx = rotation_matrix(rotatev.x, x3f);
    rotatemy = rotation_matrix(rotatev.y, y3f);
    rotatemz = rotation_matrix(rotatev.z, z3f);
    transm = translation_matrix(transv);

    // compute combined xform matrix, order matters
    transform = transm*rotatemz*rotatemy*rotatemx;
    
    //translate rest_frame by this transform matrix
    frame1 = animation->rest_frame;
    framem = frame_to_matrix(frame1);
    resultm = framem*transform;
    framenew = matrix_to_frame(resultm);

    // return the transformed rest frame
    return framenew;
}

// update mesh frames for animation 3)
void animate_frame(Scene* scene) {
    
    // foreach mesh
    for (auto mesh : scene->meshes) {
        // if not animation, continue
        if (mesh->animation  == nullptr) continue;
            // call animate_compute_frame and update mesh frame
            mesh->frame = animate_compute_frame(mesh->animation, scene->animation->time);
        }
    // for each surface
    for (auto surface: scene->surfaces) {
        // if not animation, continue
        if (surface->animation  == nullptr) continue;
            // call animate_compute_frame and update surface frame
            surface->frame = animate_compute_frame(surface->animation, scene->animation->time);
            // update the _display_mesh frame if exists
            if (surface->_display_mesh != nullptr) surface->_display_mesh->frame = animate_compute_frame(surface->animation, scene->animation->time);
    }
}

// skinning scene 4)
void animate_skin(Scene* scene) {
    
    // foreach mesh
     for (auto mesh : scene->meshes) {
        // if no skinning, continue
        if (mesh->skinning  == nullptr) continue;

        // foreach vertex index
        for (int i = 0; i < mesh->pos.size(); i++) {
            // set pos/norm to zero
            mesh->pos[i]=zero3f;
            mesh->norm[i]=zero3f;

            // for each bone slot (0..3)
            for (int j = 0; j < 4; j++) {
               // get bone weight and index
                auto idx = mesh->skinning->bone_ids[i][j];
                auto wght = mesh->skinning->bone_weights[i][j];
                // if index < 0, continue
                if (idx < 0) continue;
                // grab bone xform
                auto matrix = mesh->skinning->bone_xforms[scene->animation->time][idx];

                // accumulate weighted and transformed rest position and normal
                auto newpoint = transform_point(matrix, mesh->skinning->rest_pos[i]);
                mesh->pos[i] = mesh->pos[i] + newpoint*wght;
                auto newnorm = transform_normal(matrix, mesh->skinning->rest_norm[i]);
                mesh->norm[i] = mesh->norm[i] + newnorm*wght;
            }
            // normalize normal
            mesh->norm[i] = normalize(mesh->norm[i]);
        }
     }
}

// particle simulation 5)
void simulate(Scene* scene) {
    auto t = 0.0f; //time in between steps

    // for each mesh
    for (auto mesh : scene->meshes){
        // skip if no simulation
        if (mesh->simulation == nullptr) continue;
        // compute time per step
        t = ((float)scene->animation->dt)/((float)scene->animation->simsteps);

        // foreach simulation steps
        for (int i = 0; i < scene->animation->simsteps; i++) {

            //zero them all first
            for (int j = 0; j < mesh->pos.size(); j++) {
                mesh->simulation->force[j] = zero3f;

            }
            // foreach particle, compute external forces
            for (int j = 0; j < mesh->pos.size(); j++){
                vec3f gravforce = (scene->animation->gravity)*mesh->simulation->mass[j];
                // accumulate sum of forces on particle //only gravity
                mesh->simulation->force[j] = mesh->simulation->force[j] + gravforce;
        

            }
            // for each spring, compute spring force on points
             for (auto spring : mesh->simulation->springs) {
                // compute spring distance and length
                auto id1 = spring.ids.x;
                auto id2 = spring.ids.y;
                auto slength = dist(mesh->pos[id1],mesh->pos[id2]);
                auto direction = (mesh->pos[id1]-mesh->pos[id2]);
                auto sdis = (slength - spring.restlength); //displacement

                // compute static force
                auto statforce = sdis*-(spring.ks);

                // compute dynamic force
                auto v1 = mesh->simulation->vel[id1];
                auto v2 = mesh->simulation->vel[id2];
                auto dynforce = (spring.kd)*(dot((v1-v2),normalize(direction)));
                // accumulate dynamic force on points
                mesh->simulation->force[id1] += (statforce-dynforce)*direction;
                mesh->simulation->force[id2] -= (statforce-dynforce)*direction;

             }
            // foreach particle, integrate using newton laws
            for (int j = 0; j < mesh->simulation->init_pos.size(); j++) {
                // if pinned, skip
                if (mesh->simulation->pinned[j]) continue;
                // compute acceleration
                auto accel = mesh->simulation->force[j]/mesh->simulation->mass[j];

                // update velocity and positions using Euler's method
                mesh->simulation->vel[j] += accel*t;
                mesh->pos[j]+= mesh->simulation->vel[j]*t;

                // for each surface, check for collision
                for (auto surface : scene->surfaces) {
                    //establish radius, position point, normal, and inside switch
                    auto r = surface->radius;
                    vec3f newpos;
                    vec3f norm;
                    bool inside = false;

                    // if quad
                    if (surface->isquad) {
                        // compute local position
                        auto worldp = mesh->pos[j];
                        auto p = transform_point_inverse(surface->frame, worldp);
                        // perform inside test
                        if (abs(p.x)<r && abs(p.y)<r && p.z<0) inside = true;
                        if (inside == true) {
                            // if inside, compute a collision position and normal
                            norm = normalize(surface->frame.z);
                            newpos = p;
                            newpos.z = 0; //z component is set to 0, it's on the surface
                            //change back to world coordinates
                            newpos = transform_point_from_local(surface->frame, newpos);
                        }
                    }

                    // if sphere
                    else {
                        // inside test
                        auto p = mesh->pos[j];
                        // if inside, compute a collision position and normal
                        if (dist(p,surface->frame.o) <= r) inside = true;
                        if (inside == true) {
                            //norm of collision point
                            norm = normalize(p - surface->frame.o);
                            //position = position of collision
                            newpos = norm*r + surface->frame.o;

                        }
                    }
                    // if inside
                    if (inside == true) {
                        // set particle position
                        mesh->pos[j]= newpos;

                        // update velocity (particle bounces), taking into account loss of kinetic energy
                        auto bd = scene->animation->bounce_dump;
                        float mag = dot(mesh->simulation->vel[j], norm);
                        auto vn = mag*norm;
                        auto vt = mesh->simulation->vel[j]- vn;

                        //update vt and vn by subtracting bounce dump
                        auto newvel = (1-bd.x)*vt - (1-bd.y)*vn;
                       // message("%.3f",vel[j]);
                        mesh->simulation->vel[j] = newvel;
                    }
                    //compute neighbor
                    vector<int> neighbors;  //ids of neighboring particles of j
                    //int boundary = (1,1,1);
                    
                    //for (int k = 0; k < mesh->simulation->init_pos.size(); k++) {
                      //  if (dist(mesh->pos[j],mesh->pos[k]) < boundary) neighbors.push_back(k);
                      //  }
                    neighbors.push_back(2);
                    neighbors.push_back(5);
                    neighbors.push_back(8);
                    neighbors.push_back(9);
                    neighbors.push_back(11);
                    message("%d",neighbors[1]);
                    vec3f neighborpostotal = zero3f;
                    vec3f neighborveltotal = zero3f;
                    vec3f particlepos = mesh->pos[j];
                    vec3f particlevel = mesh->simulation->vel[j];
                    mesh->simulation->vel[j] = zero3f;
                    
                    if (neighbors.size()>1) {
                    //compute new orientation based on neighbors
                        for (auto neighbor : neighbors) {
                            auto neighborpos = mesh->pos[neighbor];
                            auto neighborvel = mesh->simulation->vel[neighbor];
                            //add up position of all neighbors
                            neighborpostotal = neighborpostotal + neighborpos;
                            //add up velocities of all neighbors
                            neighborveltotal = neighborveltotal + neighborvel;
                        }
                    
                        //compute overall separation: added positions - #neighbors*currentpos
                        vec3f sepo = neighborpostotal - neighbors.size()*particlepos;
                        //compute overall align: average all velocity directions
                        vec3f aligno = normalize(neighborveltotal/neighbors.size());
                        //compute overall cohesion: average all positions
                        vec3f averagepos = neighborpostotal/neighbors.size();
                        vec3f coheso = averagepos - particlepos;
                        
                        //average the 3 and update velocity
                        //new velocity orientation = the average of these 3 effects;
                        vec3f newvelo = normalize((sepo+aligno+coheso)/3);
                        mesh->simulation->vel[j] += newvelo;
                        
                        }
                    }
                }
            }
    
        // smooth normals if it has triangles or quads
        if ((!mesh->triangle.empty()) || (!mesh->quad.empty())) smooth_normals(mesh);
    }
}
// scene reset
void animate_reset(Scene* scene) {
    scene->animation->time = 0;
    for(auto mesh : scene->meshes) {
        if(mesh->animation) {
            mesh->frame = mesh->animation->rest_frame;
        }
        if(mesh->skinning) {
            mesh->pos = mesh->skinning->rest_pos;
            mesh->norm = mesh->skinning->rest_norm;
        }
        if(mesh->simulation) {
            mesh->pos = mesh->simulation->init_pos;
            mesh->simulation->vel = mesh->simulation->init_vel;
            mesh->simulation->force.resize(mesh->simulation->init_pos.size());
        }
    }
}

// scene update
void animate_update(Scene* scene, bool skinning_gpu) {
    scene->animation->time ++;
    if(scene->animation->time >= scene->animation->length) animate_reset(scene);
    animate_frame(scene);
    if(not skinning_gpu) animate_skin(scene);
    simulate(scene);
}




// main function
int main(int argc, char** argv) {
    auto args = parse_cmdline(argc, argv,
        { "03_animate", "view scene",
            {  {"resolution", "r", "image resolution", typeid(int), true, jsonvalue() }  },
            {  {"scene_filename", "", "scene filename", typeid(string), false, jsonvalue("scene.json")},
               {"image_filename", "", "image filename", typeid(string), true, jsonvalue("")}  }
        });
    
    // generate/load scene either by creating a test scene or loading from json file
    scene_filename = args.object_element("scene_filename").as_string();
    scene = nullptr;
    if(scene_filename.length() > 9 and scene_filename.substr(0,9) == "testscene") {
        int scene_type = atoi(scene_filename.substr(9).c_str());
        scene = create_test_scene(scene_type);
        scene_filename = scene_filename + ".json";
    } else {
        scene = load_json_scene(scene_filename);
    }
    error_if_not(scene, "scene is nullptr");
    
    image_filename = (args.object_element("image_filename").as_string() != "") ?
        args.object_element("image_filename").as_string() :
        scene_filename.substr(0,scene_filename.size()-5)+".png";
    
    if(not args.object_element("resolution").is_null()) {
        scene->image_height = args.object_element("resolution").as_int();
        scene->image_width = scene->camera->width * scene->image_height / scene->camera->height;
    }
    
    animate_reset(scene);
    
    subdivide(scene);
    
    uiloop();
}





/////////////////////////////////////////////////////////////////////
// UI and Rendering Code: OpenGL, GLFW, GLSL


bool save         = false;      // whether to start the save loop
bool animate      = false;      // run animation
bool draw_faces   = true;       // draw faces of mesh
bool draw_lines   = true;       // draw lines/splines of mesh
bool draw_points  = true;       // draw points of mesh
bool draw_edges   = false;      // draw edges of mesh
bool draw_normals = false;      // draw normals

bool skinning_gpu = false;      // skinning on the gpu    NOTE: NOT USED!

int gl_program_id = 0;          // OpenGL program handle
int gl_vertex_shader_id = 0;    // OpenGL vertex shader handle
int gl_fragment_shader_id = 0;  // OpenGL fragment shader handle
map<image3f*,int> gl_texture_id;// OpenGL texture handles

// initialize the shaders
void init_shaders() {
    // load shader code from files
    auto vertex_shader_code    = load_text_file("../../../scenes/animate_vertex.glsl");
    auto fragment_shader_code  = load_text_file("../../../scenes/animate_fragment.glsl");
    auto vertex_shader_codes   = (char *)vertex_shader_code.c_str();
    auto fragment_shader_codes = (char *)fragment_shader_code.c_str();

    // create shaders
    gl_vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    gl_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    
    // load shaders code onto the GPU
    glShaderSource(gl_vertex_shader_id,1,(const char**)&vertex_shader_codes,nullptr);
    glShaderSource(gl_fragment_shader_id,1,(const char**)&fragment_shader_codes,nullptr);
    
    // compile shaders
    glCompileShader(gl_vertex_shader_id);
    glCompileShader(gl_fragment_shader_id);
    
    // check if shaders are valid
    error_if_glerror();
    error_if_shader_not_valid(gl_vertex_shader_id);
    error_if_shader_not_valid(gl_fragment_shader_id);
    
    // create program
    gl_program_id = glCreateProgram();
    
    // attach shaders
    glAttachShader(gl_program_id,gl_vertex_shader_id);
    glAttachShader(gl_program_id,gl_fragment_shader_id);
    
    // bind vertex attributes locations
    glBindAttribLocation(gl_program_id, 0, "vertex_pos");
    glBindAttribLocation(gl_program_id, 1, "vertex_norm");
    glBindAttribLocation(gl_program_id, 2, "vertex_texcoord");
    glBindAttribLocation(gl_program_id, 3, "vertex_skin_bones");
    glBindAttribLocation(gl_program_id, 4, "vertex_skin_weights");

    // link program
    glLinkProgram(gl_program_id);
    
    // check if program is valid
    error_if_glerror();
    error_if_program_not_valid(gl_program_id);
}

// initialize the textures
void init_textures(Scene* scene) {
    // grab textures from scene
    auto textures = get_textures(scene);
    // foreach texture
    for(auto texture : textures) {
        // if already in the gl_texture_id map, skip
        if(gl_texture_id.find(texture) != gl_texture_id.end()) continue;
        // gen texture id
        unsigned int id = 0;
        glGenTextures(1, &id);
        // set id to the gl_texture_id map for later use
        gl_texture_id[texture] = id;
        // bind texture
        glBindTexture(GL_TEXTURE_2D, id);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        // load texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     texture->width(), texture->height(),
                     0, GL_RGB, GL_FLOAT, texture->data());
    }
}

// utility to bind texture parameters for shaders
// uses texture name, texture_on name, texture pointer and texture unit position
void _bind_texture(string name_map, string name_on, image3f* txt, int pos) {
    // if txt is not null
    if(txt) {
        // set texture on boolean parameter to true
        glUniform1i(glGetUniformLocation(gl_program_id,name_on.c_str()),GL_TRUE);
        // activate a texture unit at position pos
        glActiveTexture(GL_TEXTURE0+pos);
        // bind texture object to it from gl_texture_id map
        glBindTexture(GL_TEXTURE_2D, gl_texture_id[txt]);
        // set texture parameter to the position pos
        glUniform1i(glGetUniformLocation(gl_program_id, name_map.c_str()), pos);
    } else {
        // set texture on boolean parameter to false
        glUniform1i(glGetUniformLocation(gl_program_id,name_on.c_str()),GL_FALSE);
        // activate a texture unit at position pos
        glActiveTexture(GL_TEXTURE0+pos);
        // set zero as the texture id
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

// shade a mesh
void shade_mesh(Mesh* mesh, int time) {
    // bind material kd, ks, n
    glUniform3fv(glGetUniformLocation(gl_program_id,"material_kd"),
                 1,&mesh->mat->kd.x);
    glUniform3fv(glGetUniformLocation(gl_program_id,"material_ks"),
                 1,&mesh->mat->ks.x);
    glUniform1f(glGetUniformLocation(gl_program_id,"material_n"),
                mesh->mat->n);
    glUniform1i(glGetUniformLocation(gl_program_id,"material_is_lines"),
                GL_FALSE);
    glUniform1i(glGetUniformLocation(gl_program_id,"material_double_sided"),
                (mesh->mat->double_sided)?GL_TRUE:GL_FALSE);
    // bind texture params (txt_on, sampler)
    _bind_texture("material_kd_txt", "material_kd_txt_on", mesh->mat->kd_txt, 0);
    _bind_texture("material_ks_txt", "material_ks_txt_on", mesh->mat->ks_txt, 1);
    _bind_texture("material_norm_txt", "material_norm_txt_on", mesh->mat->norm_txt, 2);
    
    // bind mesh frame - use frame_to_matrix
    glUniformMatrix4fv(glGetUniformLocation(gl_program_id,"mesh_frame"),
                       1,true,&frame_to_matrix(mesh->frame)[0][0]);
    
    // enable vertex attributes arrays and set up pointers to the mesh data
    auto vertex_pos_location = glGetAttribLocation(gl_program_id, "vertex_pos");
    auto vertex_norm_location = glGetAttribLocation(gl_program_id, "vertex_norm");
    auto vertex_texcoord_location = glGetAttribLocation(gl_program_id, "vertex_texcoord");
    auto vertex_skin_bone_ids_location = glGetAttribLocation(gl_program_id, "vertex_skin_bone_ids");
    auto vertex_skin_bone_weights_location = glGetAttribLocation(gl_program_id, "vertex_skin_bone_weights");
    
    glEnableVertexAttribArray(vertex_pos_location);
    glVertexAttribPointer(vertex_pos_location, 3, GL_FLOAT, GL_FALSE, 0, &mesh->pos[0].x);
    glEnableVertexAttribArray(vertex_norm_location);
    glVertexAttribPointer(vertex_norm_location, 3, GL_FLOAT, GL_FALSE, 0, &mesh->norm[0].x);
    if(not mesh->texcoord.empty()) {
        glEnableVertexAttribArray(vertex_texcoord_location);
        glVertexAttribPointer(vertex_texcoord_location, 2, GL_FLOAT, GL_FALSE, 0, &mesh->texcoord[0].x);
    }
    else glVertexAttrib2f(vertex_texcoord_location, 0, 0);
    
    if (mesh->skinning and skinning_gpu) {
        glUniform1i(glGetUniformLocation(gl_program_id,"skinning->enabled"),GL_TRUE);
        glUniformMatrix4fv(glGetUniformLocation(gl_program_id,"skinning->bone_xforms"),
                           mesh->skinning->bone_xforms[time].size(), GL_TRUE,
                           &mesh->skinning->bone_xforms[time][0].x.x);
        glEnableVertexAttribArray(vertex_skin_bone_ids_location);
        glEnableVertexAttribArray(vertex_skin_bone_weights_location);
        glVertexAttribPointer(vertex_skin_bone_ids_location, 4, GL_INT, GL_FALSE, 0, mesh->skinning->bone_ids.data());
        glVertexAttribPointer(vertex_skin_bone_weights_location, 4, GL_FLOAT, GL_FALSE, 0, mesh->skinning->bone_weights.data());
    } else {
        glUniform1i(glGetUniformLocation(gl_program_id,"skinning->enabled"),GL_FALSE);
    }
    
    // draw triangles and quads
    if(draw_faces) {
        if(mesh->triangle.size()) glDrawElements(GL_TRIANGLES, mesh->triangle.size()*3, GL_UNSIGNED_INT, &mesh->triangle[0].x);
        if(mesh->quad.size())     glDrawElements(GL_QUADS, mesh->quad.size()*4, GL_UNSIGNED_INT, &mesh->quad[0].x);
    }
    
    if(draw_points) {
        if(mesh->point.size()) glDrawElements(GL_POINTS, mesh->point.size(), GL_UNSIGNED_INT, &mesh->point[0]);
    }
    
    if(draw_lines) {
        if(mesh->line.size()) glDrawElements(GL_LINES, mesh->line.size(), GL_UNSIGNED_INT, &mesh->line[0].x);
        for(auto segment : mesh->spline) glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_INT, &segment);
    }
    
    if(draw_edges) {
        auto edges = EdgeMap(mesh->triangle, mesh->quad).edges();
        glDrawElements(GL_LINES, edges.size()*2, GL_UNSIGNED_INT, &edges[0].x);
    }
    
    // disable vertex attribute arrays
    glDisableVertexAttribArray(vertex_pos_location);
    glDisableVertexAttribArray(vertex_norm_location);
    if(not mesh->texcoord.empty()) glDisableVertexAttribArray(vertex_texcoord_location);
    if(mesh->skinning) {
        glDisableVertexAttribArray(vertex_skin_bone_ids_location);
        glDisableVertexAttribArray(vertex_skin_bone_weights_location);
    }
    
    // draw normals if needed
    if(draw_normals) {
        glUniform3fv(glGetUniformLocation(gl_program_id,"material_kd"),
                     1,&zero3f.x);
        glUniform3fv(glGetUniformLocation(gl_program_id,"material_ks"),
                     1,&zero3f.x);
        glBegin(GL_LINES);
        for(auto i : range(mesh->pos.size())) {
            auto p0 = mesh->pos[i];
            auto p1 = mesh->pos[i] + mesh->norm[i]*0.1;
            glVertexAttrib3fv(0,&p0.x);
            glVertexAttrib3fv(0,&p1.x);
            if(mesh->mat->double_sided) {
                auto p2 = mesh->pos[i] - mesh->norm[i]*0.1;
                glVertexAttrib3fv(0,&p0.x);
                glVertexAttrib3fv(0,&p2.x);
            }
        }
        glEnd();
    }
}

// render the scene with OpenGL
void shade(Scene* scene) {
    // enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // disable culling face
    glDisable(GL_CULL_FACE);
    // let the shader control the points
    glEnable(GL_POINT_SPRITE);
    
    // set up the viewport from the scene image size
    glViewport(0, 0, scene->image_width, scene->image_height);
    
    // clear the screen (both color and depth) - set cleared color to background
    glClearColor(scene->background.x, scene->background.y, scene->background.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // enable program
    glUseProgram(gl_program_id);
    
    // bind camera's position, inverse of frame and projection
    // use frame_to_matrix_inverse and frustum_matrix
    glUniform3fv(glGetUniformLocation(gl_program_id,"camera_pos"),
                 1, &scene->camera->frame.o.x);
    glUniformMatrix4fv(glGetUniformLocation(gl_program_id,"camera_frame_inverse"),
                       1, true, &frame_to_matrix_inverse(scene->camera->frame)[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(gl_program_id,"camera_projection"),
                       1, true, &frustum_matrix(-scene->camera->dist*scene->camera->width/2, scene->camera->dist*scene->camera->width/2,
                                                -scene->camera->dist*scene->camera->height/2, scene->camera->dist*scene->camera->height/2,
                                                scene->camera->dist,10000)[0][0]);
    
    // bind ambient and number of lights
    glUniform3fv(glGetUniformLocation(gl_program_id,"ambient"),1,&scene->ambient.x);
    glUniform1i(glGetUniformLocation(gl_program_id,"lights_num"),scene->lights.size());
    
    // foreach light
    auto count = 0;
    for(auto light : scene->lights) {
        // bind light position and internsity (create param name with tostring)
        glUniform3fv(glGetUniformLocation(gl_program_id,tostring("light_pos[%d]",count).c_str()),
                     1, &light->frame.o.x);
        glUniform3fv(glGetUniformLocation(gl_program_id,tostring("light_intensity[%d]",count).c_str()),
                     1, &light->intensity.x);
        count++;
    }
    
    // foreach mesh
    for(auto mesh : scene->meshes) {
        // draw mesh
        shade_mesh(mesh, scene->animation->time);
    }
    
    // foreach surface
    for(auto surface : scene->surfaces) {
        // draw display mesh
        shade_mesh(surface->_display_mesh, scene->animation->time);
    }
}


// uiloop
void uiloop() {
    
    auto ok_glfw = glfwInit();
    error_if_not(ok_glfw, "glfw init error");
    
    // setting an error callback
    glfwSetErrorCallback([](int ecode, const char* msg){ return error(msg); });
    
    glfwWindowHint(GLFW_SAMPLES, scene->image_samples);

    auto window = glfwCreateWindow(scene->image_width, scene->image_height,
                                   "graphics | animate", NULL, NULL);
    error_if_not(window, "glfw window error");
    
    glfwMakeContextCurrent(window);
    
    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int key) {
        switch (key) {
            case 's': { save = true; } break;
            case ' ': { animate = not animate; } break;
            case '.': { animate_update(scene, skinning_gpu); } break;
            case 'g': { skinning_gpu = not skinning_gpu; animate_reset(scene); } break;
            case 'n': { draw_normals = not draw_normals; } break;
            case 'e': { draw_edges = not draw_edges; } break;
            case 'p': { draw_points = not draw_points; } break;
            case 'f': { draw_faces = not draw_faces; } break;
        }
    });
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    auto ok_glew = glewInit();
    error_if_not(GLEW_OK == ok_glew, "glew init error");
    
    init_shaders();
    init_textures(scene);
    animate_reset(scene);
    
    auto mouse_last_x = -1.0;
    auto mouse_last_y = -1.0;
    
    auto last_update_time = glfwGetTime();
    
    while(not glfwWindowShouldClose(window)) {
        auto title = tostring("graphics | animate | %03d", scene->animation->time);
        glfwSetWindowTitle(window, title.c_str());
        
        if(animate) {
            if(glfwGetTime() - last_update_time > scene->animation->dt) {
                last_update_time = glfwGetTime();
                animate_update(scene, skinning_gpu);
            }
        }
        
        if(save) {
            animate_reset(scene);
            for(auto i : range(scene->animation->length/3)) animate_update(scene, skinning_gpu);
        }
        
        glfwGetFramebufferSize(window, &scene->image_width, &scene->image_height);
        scene->camera->width = (scene->camera->height * scene->image_width) / scene->image_height;
        
        shade(scene);

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            if (mouse_last_x < 0 or mouse_last_y < 0) { mouse_last_x = x; mouse_last_y = y; }
            auto delta_x = x - mouse_last_x, delta_y = y - mouse_last_y;
            
            set_view_turntable(scene->camera, delta_x*0.01, -delta_y*0.01, 0, 0, 0);
            
            mouse_last_x = x;
            mouse_last_y = y;
        } else { mouse_last_x = -1; mouse_last_y = -1; }
        
        if(save) {
            auto image = image3f(scene->image_width,scene->image_height);
            glReadPixels(0, 0, scene->image_width, scene->image_height, GL_RGB, GL_FLOAT, &image.at(0,0));
            write_png(image_filename, image, true);
            save = false;
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
}


