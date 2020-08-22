#include "src/DGNEngine/DGNEngine.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define ASSERT_RETURN(x) if(!(x)) return 1

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 680

#define CASCADE_COUNT 3
#define SHADOW_SIZE 1024
#define SHADOW_FAR 33.0f
#define SHADOW_NEAR 0.1f
#define CASCADE_SPLIT_BLEND 0.5f

#include "src/c_ordered_map.h"
#include "src/c_linked_list.h"

void updateCamera(DgnCamera *camera, DgnWindow *window, uint8_t controller);

int main(int argc, char* argv[])
{
    DgnWindow *window = NULL;
    dgnWindowCreate(&window, WINDOW_WIDTH, WINDOW_HEIGHT, "Platformer");
    ASSERT_RETURN(window != NULL);
    dgnWindowMakeCurrent(window);

    dgnRendererInitialize();
    dgnRendererEnableFlag(DGN_RENDER_FLAG_DEPTH_TEST);
    dgnRendererEnableFlag(DGN_RENDER_FLAG_CULL_FACE);
    dgnRendererEnableFlag(DGN_RENDER_FLAG_SEAMLESS_CUBEMAP);

    /** ---------------- GAME SETUP ---------------- **/

    /** -------- CAMERA -------- **/

    DgnCamera camera;
    camera.frustum.far    = 100;
    camera.frustum.near   = 0.1;
    camera.frustum.fov          = 90 * TO_RADS;
    camera.frustum.height       = WINDOW_HEIGHT;
    camera.frustum.width        = WINDOW_WIDTH;
    camera.pos                  = (Vec3){0.0f, 1.0f, 3.0f};
    camera.rot                  = (Quat){0.0f, 0.0f, 0.0f, 1.0f};

    /** -------- SHADOW CASCADES -------- **/

    DgnShadowMap shadow_cascades[CASCADE_COUNT];

    for(int i = 0; i < CASCADE_COUNT; i++)
    {
        uint8_t att = DGN_FRAMEBUFFER_DEPTH;
        DgnTexture *map = dgnLightingCreateShadowMap(SHADOW_SIZE, SHADOW_SIZE, DGN_LIGHT_TYPE_DIR);
        shadow_cascades[i].texture = map;
        shadow_cascades[i].framebuffer = dgnFramebufferCreate(&map, &att, 1, DGN_FRAMEBUFFER_DEPTH);
        shadow_cascades[i].proj_mat = m3dMat4x4InitIdentity();
        shadow_cascades[i].view_mat = m3dMat4x4InitIdentity();
    }

    int cascade_depths_count = CASCADE_COUNT + 1;
    float cascade_depths[cascade_depths_count];

    for(int i = 0; i < cascade_depths_count; i++)
    {
        float ioverm = (float)i / (float)CASCADE_COUNT;
        float dist_uni = SHADOW_NEAR + (SHADOW_FAR - SHADOW_NEAR) * ioverm;
        float dist_log = SHADOW_NEAR * pow(SHADOW_FAR / SHADOW_NEAR, ioverm);

        cascade_depths[i] = m3d1DLerp(dist_log, dist_uni, CASCADE_SPLIT_BLEND);
    }

    /** -------- SCREEN FRAMEBUFFER -------- **/

    uint8_t screen_attachement = DGN_FRAMEBUFFER_COLOR;
    DgnTexture *screen_texture = dgnTextureCreate(NULL, WINDOW_WIDTH, WINDOW_HEIGHT, DGN_TEX_WRAP_CLAMP_TO_EDGE,
                                                  DGN_TEX_FILTER_BILINEAR, DGN_FALSE, DGN_TEX_STORAGE_RGBA, DGN_TEX_STORAGE_RGBA16F,
                                                  DGN_DATA_TYPE_FLOAT);
    DgnFramebuffer *screen_framebuffer = dgnFramebufferCreate(&screen_texture, &screen_attachement, 1, DGN_FRAMEBUFFER_DEPTH);

    /** -------- ASSETS -------- **/

    const char *skybox_locations[] =
    {
        "res/game/skyboxday/right.png",
        "res/game/skyboxday/left.png",
        "res/game/skyboxday/up.png",
        "res/game/skyboxday/down.png",
        "res/game/skyboxday/back.png",
        "res/game/skyboxday/front.png"
    };

    uint16_t level_mesh_count = 0;
    DgnMesh **level_mesh = NULL;
    DgnMesh **ball_mesh = NULL;

    DgnTexture *skybox_texture = NULL;
    DgnTexture *checker_textures[4] = {NULL, NULL, NULL, NULL};
    DgnTexture *ball_texture = NULL;
    DgnTexture *toon_map = NULL;
    DgnTexture *toon_map2 = NULL;

    DgnShader *skybox_shader = NULL;
    DgnShader *lit_shader = NULL;
    DgnShader *screen_shader = NULL;
    DgnShader *shadow_shader = NULL;
    DgnShader *color_shader = NULL;
    DgnShader *line_shader = NULL;

    dgnShaderSetEconstI("NUM_CASCADES", CASCADE_COUNT);

    ASSERT_RETURN(level_mesh = dgnMeshLoad("res/game/test_level_1.obj", &level_mesh_count));
    //ASSERT_RETURN(ball_mesh = dgnMeshLoad("res/game/ball.obj", NULL));
    ASSERT_RETURN(ball_mesh = dgnMeshLoad("res/monkey.obj", NULL));

    ASSERT_RETURN(skybox_texture = dgnCubemapLoad(skybox_locations, DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(checker_textures[0] = dgnTextureLoad("res/game/checker1.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(checker_textures[1] = dgnTextureLoad("res/game/checker2.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(checker_textures[2] = dgnTextureLoad("res/game/checker3.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(checker_textures[3] = dgnTextureLoad("res/game/checker4.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(ball_texture = dgnTextureLoad("res/game/checker5.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(toon_map = dgnTextureLoad("res/game/toonMap1.png", DGN_TEX_WRAP_CLAMP_TO_EDGE, DGN_TEX_FILTER_BILINEAR, DGN_FALSE, DGN_TEX_STORAGE_RGB));
    ASSERT_RETURN(toon_map2 = dgnTextureLoad("res/game/toonMap2.png", DGN_TEX_WRAP_CLAMP_TO_EDGE, DGN_TEX_FILTER_BILINEAR, DGN_FALSE, DGN_TEX_STORAGE_RGB));

    ASSERT_RETURN(skybox_shader = dgnShaderLoad("res/game/skybox.vert", 0, "res/game/skybox.frag"));
    //ASSERT_RETURN(lit_shader = dgnShaderLoad("res/game/shadow_viewer.vert", 0, "res/game/shadow_viewer.frag"));
    ASSERT_RETURN(lit_shader = dgnShaderLoad("res/game/lit.vert", 0, "res/game/lit.frag"));
    ASSERT_RETURN(screen_shader = dgnShaderLoad("res/game/screen.vert", 0, "res/game/screen.frag"));
    ASSERT_RETURN(shadow_shader = dgnShaderLoad("res/game/shadow.vert", 0, 0));
    ASSERT_RETURN(color_shader = dgnShaderLoad("res/game/wireframe.vert", 0, "res/game/wireframe.frag"));
    ASSERT_RETURN(line_shader = dgnShaderLoad("res/game/line.vert", 0, "res/game/wireframe.frag"));

    int skybox_u_vp = dgnShaderGetUniformLoc(skybox_shader, "uVP");
    int skybox_u_sun_dir = dgnShaderGetUniformLoc(skybox_shader, "uSunDir");

    int lit_u_model = dgnShaderGetUniformLoc(lit_shader, "uModel");
    int lit_u_view = dgnShaderGetUniformLoc(lit_shader, "uView");
    int lit_u_projection = dgnShaderGetUniformLoc(lit_shader, "uProjection");
    int lit_u_texture = dgnShaderGetUniformLoc(lit_shader, "uTexture");
    int lit_u_toon_map = dgnShaderGetUniformLoc(lit_shader, "uToonMap");
    int lit_u_toon_map2 = dgnShaderGetUniformLoc(lit_shader, "uToonMap2");
    int lit_u_has_texture = dgnShaderGetUniformLoc(lit_shader, "uHasTexture");
    int lit_u_skybox = dgnShaderGetUniformLoc(lit_shader, "uSkybox");
    int lit_u_light_dir = dgnShaderGetUniformLoc(lit_shader, "uLightDir");
    int lit_u_cam_pos = dgnShaderGetUniformLoc(lit_shader, "uCamPos");
    int lit_u_specular = dgnShaderGetUniformLoc(lit_shader, "uShininess");
    int lit_u_refl_shine = dgnShaderGetUniformLoc(lit_shader, "uReflectShininess");
    int lit_u_metalness = dgnShaderGetUniformLoc(lit_shader, "uMetalness");

    int lit_u_light_mat[CASCADE_COUNT];
    int lit_u_shadow_map[CASCADE_COUNT];
    int lit_u_cascade_ends[CASCADE_COUNT];

    for(int i = 0; i < CASCADE_COUNT; i++)
    {
        char buff[32];
        char j[1];
        itoa(i, j, 10);

        strcat(strcat(strcpy(buff, "uLightMat["), j), "]");
        lit_u_light_mat[i] = dgnShaderGetUniformLoc(lit_shader, buff);

        strcat(strcat(strcpy(buff, "uShadowMap["), j), "]");
        lit_u_shadow_map[i] = dgnShaderGetUniformLoc(lit_shader, buff);

        strcat(strcat(strcpy(buff, "uCascadeEnd["), j), "]");
        lit_u_cascade_ends[i] = dgnShaderGetUniformLoc(lit_shader, buff);
    }

    int screen_u_scale = dgnShaderGetUniformLoc(screen_shader, "uScale");
    int screen_u_offset = dgnShaderGetUniformLoc(screen_shader, "uOffset");
    int screen_u_texture = dgnShaderGetUniformLoc(screen_shader, "uTexture");
    int screen_u_single = dgnShaderGetUniformLoc(screen_shader, "uSingle");

    int shadow_u_model = dgnShaderGetUniformLoc(shadow_shader, "uModel");
    int shadow_u_light = dgnShaderGetUniformLoc(shadow_shader, "uLight");

    int color_u_color = dgnShaderGetUniformLoc(color_shader, "uColor");
    int color_u_mvp = dgnShaderGetUniformLoc(color_shader, "uMVP");

    int line_u_color = dgnShaderGetUniformLoc(line_shader, "uColor");
    int line_u_vp = dgnShaderGetUniformLoc(line_shader, "uVP");
    int line_u_pos1 = dgnShaderGetUniformLoc(line_shader, "uPositions[0]");
    int line_u_pos2 = dgnShaderGetUniformLoc(line_shader, "uPositions[1]");

    uint8_t grounded = DGN_FALSE;
    Vec3 gravity_vector = {0.0f, -9.81f, 0.0f};

    Vec3 ball_pos = {0.0f, 1.0f, 0.0f};
    Vec3 ball_velo = {0.0f, 10.0f, 0.0f};

    while(!dgnWindowShouldClose(window))
    {
        dgnInputPollEvents();

        /** ---------------- UPDATE ---------------- **/

        if(dgnInputGetKeyDown(DGN_KEY_R))
        {
            dgnShaderDestroy(lit_shader);
            dgnTextureDestroy(toon_map);
            dgnTextureDestroy(toon_map2);
            lit_shader = dgnShaderLoad("res/game/lit.vert", 0, "res/game/lit.frag");
            toon_map = dgnTextureLoad("res/game/toonMap1.png", DGN_TEX_WRAP_CLAMP_TO_EDGE, DGN_TEX_FILTER_BILINEAR, DGN_FALSE, DGN_TEX_STORAGE_RGB);
            toon_map2 = dgnTextureLoad("res/game/toonMap2.png", DGN_TEX_WRAP_CLAMP_TO_EDGE, DGN_TEX_FILTER_BILINEAR, DGN_FALSE, DGN_TEX_STORAGE_RGB);
        }


        //Vec3 sun_dir = m3dVec3Normalized(m3dQuatRotateVec3(m3dQuatAngleAxis(dgnEngineGetSeconds() / 200.0f, (Vec3){0.0f, 1.0f, 0.0f}),
        //                                 (Vec3){-1.0f, -1.0f, -1.0f}));
        Vec3 sun_dir = m3dVec3Normalized((Vec3){-1.0f, -3.0f, -1.0f});
        //Vec3 sun_dir = m3dVec3Normalized((Vec3){0.0f, 0.0f, -1.0f});

        /** -------- Ball -------- **/

        DgnBoundingSphere ball_bounds;
        ball_bounds.radius = 0.5f;
        DgnBoundingBox floor_bounds;
        floor_bounds.max = (Vec3){3.0f, 0.0f, 3.0f};
        floor_bounds.min = (Vec3){-3.0f, -1.0f, -3.0f};

        DgnTriangle tri;
        tri.p1 = (Vec3){-1.0f, 1.5f, 1.0f};
        tri.p2 = (Vec3){0.0f, 1.5f, -1.0f};
        tri.p3 = (Vec3){1.0f, 1.5f, 1.0f};

        if((dgnInputGetGamepadButton(DGN_GAMEPAD_1, DGN_GAMEPAD_BUTTON_CROSS) ||
            dgnInputGetKeyDown(DGN_KEY_SPACE)) && grounded)
        {
            ball_velo.y = 10;
            grounded = DGN_FALSE;
        }

        Mat4x4 ball_transform = m3dMat4x4InitIdentity();

        ball_velo = m3dVec3AddVec3(ball_velo, m3dVec3MulValue(gravity_vector, dgnWindowGetDelta(window)));
        if(ball_velo.y < -31.0f)
        {
            ball_velo.y = -31.0f;
        }

        //Vec3 ball_t_pos = m3dVec3AddVec3(ball_pos, m3dVec3MulValue(ball_velo, dgnWindowGetDelta(window)));
        //ball_bounds.center = ball_t_pos;

        if(dgnCollisionBoxSphere(floor_bounds, ball_bounds).hit)
        {
            //ball_t_pos = ball_pos;
            ball_velo.y = 0.0f;
            grounded = DGN_TRUE;
        }

        //ball_pos = ball_t_pos;

        if(dgnCollisionTrianglePoint(tri, ball_pos).hit)
        {
            //printf("Hit\n");
        }

        m3dMat4x4Translate(&ball_transform, ball_pos);

        /** -------- Camera -------- **/

        updateCamera(&camera, window, DGN_TRUE);
        Mat4x4 vp_mat = m3dMat4x4MulMat4x4(dgnCameraGetProjection(camera),
                        m3dMat4x4FromMat3x3(m3dMat3x3FromMat4x4(dgnCameraGetView(camera))));
        //Mat4x4 vp_mat = dgnCameraGetProjection(camera);

        /** -------- Shadows -------- **/

        DgnFrustum frustum = camera.frustum;

        for(int i = 0; i < CASCADE_COUNT; i++)
        {
            frustum.near = cascade_depths[i];
            frustum.far = cascade_depths[i + 1];

            shadow_cascades[i].view_mat = dgnLightingCreateDirViewMat(sun_dir);
            shadow_cascades[i].proj_mat = dgnLightingCreateLightProjMat(camera, shadow_cascades[i], frustum, 10.0f);
        }

        /** ---------------- RENDER ---------------- **/

        /** -------- Shadows -------- **/

        dgnRendererEnableClearFlag(DGN_CLEAR_FLAG_DEPTH);
        dgnRendererSetDepthTest(DGN_DEPTH_PASS_LESS);
        dgnRendererSetCullFace(DGN_FACE_FRONT);
        dgnRendererSetViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);

        for(int i = 0; i < CASCADE_COUNT; i++)
        {
            dgnFramebufferBind(shadow_cascades[i].framebuffer);
            dgnRendererClear();
            dgnRendererBindShader(shadow_shader);

            dgnShaderUniformM4x4(shadow_u_light, dgnLightingCreateLightSpaceMat(shadow_cascades[i]));
            dgnShaderUniformM4x4(shadow_u_model, m3dMat4x4InitIdentity());

            for(int i = 0; i < level_mesh_count; i++)
            {
                dgnRendererBindMesh(level_mesh[i]);
                dgnRendererDrawMesh();
            }

            dgnShaderUniformM4x4(shadow_u_model, ball_transform);
            dgnRendererBindMesh(ball_mesh[0]);
            dgnRendererDrawMesh();
        }
        dgnFramebufferBind(0);

        /** -------- Main Scene -------- **/

        dgnFramebufferBind(screen_framebuffer);

        dgnRendererSetViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        dgnRendererEnableClearFlag(DGN_CLEAR_FLAG_COLOR | DGN_CLEAR_FLAG_DEPTH);
        dgnRendererSetDepthTest(DGN_DEPTH_PASS_LESS);
        dgnRendererSetCullFace(DGN_FACE_BACK);
        dgnRendererClear();

        dgnRendererBindShader(lit_shader);

        dgnShaderUniformM4x4(lit_u_view, dgnCameraGetView(camera));
        dgnShaderUniformM4x4(lit_u_projection, dgnCameraGetProjection(camera));
        dgnShaderUniformV3(lit_u_light_dir, sun_dir);
        dgnShaderUniformV3(lit_u_cam_pos, camera.pos);

        for(int i = 0; i < CASCADE_COUNT; i++)
        {
            dgnShaderUniformM4x4(lit_u_light_mat[i], dgnLightingCreateLightSpaceMat(shadow_cascades[i]));
            dgnRendererBindTexture(shadow_cascades[i].texture, 20 + i);
            dgnShaderUniformI(lit_u_shadow_map[i], 20 + i);
            dgnShaderUniformF(lit_u_cascade_ends[i], cascade_depths[i + 1]);
        }

        dgnRendererBindCubemap(skybox_texture, 15);
        dgnShaderUniformI(lit_u_skybox, 15);
        dgnRendererBindTexture(toon_map, 16);
        dgnShaderUniformI(lit_u_toon_map, 16);
        dgnRendererBindTexture(toon_map2, 17);
        dgnShaderUniformI(lit_u_toon_map2, 17);

        dgnShaderUniformM4x4(lit_u_model, m3dMat4x4InitIdentity());
        dgnShaderUniformB(lit_u_has_texture, DGN_TRUE);
        dgnShaderUniformI(lit_u_texture, 0);
        dgnShaderUniformF(lit_u_specular, 5.0f);
        dgnShaderUniformF(lit_u_refl_shine, 0.1f);
        dgnShaderUniformF(lit_u_metalness, 0.0f);
        for(int i = 0; i < level_mesh_count; i++)
        {
            if(i < 4)
            {
                dgnRendererBindTexture(checker_textures[i], 0);
            }
            dgnRendererBindMesh(level_mesh[i]);
            dgnRendererDrawMesh();
        }

        dgnShaderUniformB(lit_u_has_texture, DGN_TRUE);
        dgnShaderUniformI(lit_u_texture, 0);
        dgnRendererBindTexture(ball_texture, 0);
        dgnShaderUniformF(lit_u_specular, 5.0f);
        dgnShaderUniformF(lit_u_refl_shine, 0.2f);
        dgnShaderUniformF(lit_u_metalness, 0.0f);

        dgnShaderUniformM4x4(lit_u_model, ball_transform);
        dgnRendererBindMesh(ball_mesh[0]);
        dgnRendererDrawMesh();

        dgnRendererBindMesh(0);

        /** ---- SKYBOX ---- **/
        dgnRendererSetDepthTest(DGN_DEPTH_PASS_LEQUAL);
        dgnRendererBindShader(skybox_shader);
        dgnRendererBindCubemap(skybox_texture, 0);

        dgnShaderUniformM4x4(skybox_u_vp, vp_mat);
        dgnShaderUniformV3(skybox_u_sun_dir, sun_dir);

        dgnRendererBindSkybox();
        dgnRendererDrawMesh();

        /** ---- Wire Frame ---- **/

        /*dgnRendererSetDepthTest(DGN_DEPTH_PASS_ALWAYS);
        dgnRendererSetDrawMode(DGN_DRAW_MODE_LINES);
        dgnRendererSetLineWidth(2.0f);

        dgnRendererBindShader(color_shader);
        dgnShaderUniformV3(color_u_color, (Vec3){1.0f, 0.0f, 0.0f});

        Mat4x4 wf_VP = m3dMat4x4MulMat4x4(dgnCameraGetProjection(camera), dgnCameraGetView(camera));

        dgnShaderUniformM4x4(color_u_mvp, m3dMat4x4MulMat4x4(wf_VP, dgnCollisionBoxGetModel(floor_bounds)));
        dgnRendererBindWireCube();
        dgnRendererDrawMesh();

        dgnShaderUniformM4x4(color_u_mvp, m3dMat4x4MulMat4x4(wf_VP, dgnCollisionSphereGetModel(ball_bounds)));
        dgnRendererBindWireSphere();
        dgnRendererDrawMesh();

        dgnRendererBindShader(line_shader);
        dgnShaderUniformV3(line_u_color, (Vec3){1.0f, 0.0f, 0.0f});
        dgnShaderUniformM4x4(line_u_vp, wf_VP);
        dgnRendererBindLine();

        dgnShaderUniformV3(line_u_pos1, tri.p1);
        dgnShaderUniformV3(line_u_pos2, tri.p2);
        dgnRendererDrawMesh();

        dgnShaderUniformV3(line_u_pos1, tri.p2);
        dgnShaderUniformV3(line_u_pos2, tri.p3);
        dgnRendererDrawMesh();

        dgnShaderUniformV3(line_u_pos1, tri.p3);
        dgnShaderUniformV3(line_u_pos2, tri.p1);
        dgnRendererDrawMesh();

        dgnRendererSetDrawMode(DGN_DRAW_MODE_TRIANGLES);*/

        /** ---- Screen quad ---- **/

        dgnFramebufferBind(0);

        dgnRendererSetViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        dgnRendererSetDepthTest(DGN_DEPTH_PASS_ALWAYS);
        dgnRendererBindShader(screen_shader);

        dgnShaderUniformB(screen_u_single, DGN_FALSE);
        dgnShaderUniformV2(screen_u_scale, (Vec2){1.0f, 1.0f});
        dgnShaderUniformV2(screen_u_offset, (Vec2){0.0f, 0.0f});

        dgnRendererBindTexture(screen_texture, 0);

        dgnRendererBindScreenTexture();
        dgnRendererDrawMesh();

        for(int i = 0; i < CASCADE_COUNT; i++)
        {
            float cc_inverse = 1.0f / 10;

            dgnShaderUniformB(screen_u_single, DGN_TRUE);
            dgnShaderUniformV2(screen_u_scale, (Vec2){cc_inverse, cc_inverse * (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT});

            dgnShaderUniformV2(screen_u_offset, (Vec2){ i * 2.0f * cc_inverse, 2.0f - 2.0f * cc_inverse * (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT});

            dgnRendererBindTexture(shadow_cascades[i].texture, 0);

            dgnRendererDrawMesh();
        }

        dgnWindowSwapBuffers(window);
    }

    dgnMeshDestroyArr(level_mesh, level_mesh_count);
    dgnMeshDestroyArr(ball_mesh, 1);

    dgnTextureDestroy(skybox_texture);
    dgnTextureDestroy(checker_textures[0]);
    dgnTextureDestroy(checker_textures[1]);
    dgnTextureDestroy(checker_textures[2]);
    dgnTextureDestroy(checker_textures[3]);

    dgnShaderDestroy(skybox_shader);
    dgnShaderDestroy(lit_shader);

    dgnRendererTerminate();
    dgnWindowDestroy(window);
    dgnEngineTerminate();
}


uint8_t cam_lock = DGN_FALSE;

void updateCamera(DgnCamera *camera, DgnWindow *window, uint8_t controller)
{
    float camera_rot_speed = PI * dgnWindowGetDelta(window);
    float camera_move_speed = 3.0f * dgnWindowGetDelta(window);
    Quat camera_rot_y;
    Quat camera_rot_x;

    if(controller)
    {
        camera_rot_y = m3dQuatAngleAxis(
                        dgnInputGetGamepadAxis(DGN_GAMEPAD_1, DGN_GAMEPAD_AXIS_RIGHT_X, 0.1f) * camera_rot_speed,
                        (Vec3){0.0f, -1.0f, 0.0f});

        camera_rot_x = m3dQuatAngleAxis(
                        dgnInputGetGamepadAxis(DGN_GAMEPAD_1, DGN_GAMEPAD_AXIS_RIGHT_Y, 0.1f) * camera_rot_speed,
                        m3dQuatRotateVec3(camera->rot, (Vec3){-1.0f, 0.0f, 0.0f}));

        camera->rot = m3dQuatMulQuat(m3dQuatMulQuat(camera_rot_y, camera_rot_x), camera->rot);

        camera->pos = m3dVec3AddVec3(camera->pos,
                                    m3dQuatRotateVec3(camera->rot, (Vec3){0.0f, 0.0f,
                                    camera_move_speed * dgnInputGetGamepadAxis(DGN_GAMEPAD_1, DGN_GAMEPAD_AXIS_LEFT_Y, 0.1f)}));

        camera->pos = m3dVec3AddVec3(camera->pos,
                                m3dQuatRotateVec3(camera->rot, (Vec3){
                                camera_move_speed * dgnInputGetGamepadAxis(DGN_GAMEPAD_1, DGN_GAMEPAD_AXIS_LEFT_X, 0.1f),
                                0.0f, 0.0f}));
    }
    else
    {
        camera_rot_speed = PI * 0.004f;

        // Mouse and keyboard
        if(dgnInputGetKeyDown(DGN_KEY_ESCAPE))
        {
            cam_lock = !cam_lock;

            if(cam_lock)
            {
                dgnWindowSetRawCursorMode(window, DGN_TRUE);
                dgnWindowSetCursorMode(window, DGN_CURSOR_DISABLED);
            }
            else
            {
                dgnWindowSetRawCursorMode(window, DGN_FALSE);
                dgnWindowSetCursorMode(window, DGN_CURSOR_NORMAL);
            }
        }

        if(cam_lock)
        {
            camera_rot_y = m3dQuatAngleAxis(
                        dgnInputGetMouseXDelta() * camera_rot_speed,
                        (Vec3){0.0f, -1.0f, 0.0f});

            camera_rot_x = m3dQuatAngleAxis(
                        dgnInputGetMouseYDelta() * camera_rot_speed,
                        m3dQuatRotateVec3(camera->rot, (Vec3){-1.0f, 0.0f, 0.0f}));
            camera->rot = m3dQuatMulQuat(m3dQuatMulQuat(camera_rot_y, camera_rot_x), camera->rot);
        }


        camera->pos = m3dVec3AddVec3(camera->pos,
                                    m3dQuatRotateVec3(camera->rot, (Vec3){0.0f, 0.0f,
                                    camera_move_speed * (dgnInputGetKey(DGN_KEY_S) - dgnInputGetKey(DGN_KEY_W))}));

        camera->pos = m3dVec3AddVec3(camera->pos,
                                    m3dQuatRotateVec3(camera->rot, (Vec3){
                                    camera_move_speed * (dgnInputGetKey(DGN_KEY_D) - dgnInputGetKey(DGN_KEY_A)),
                                    0.0f, 0.0f}));
    }
}

//TODO: triangle collision https://gdbooks.gitbooks.io/3dcollisions/content/Chapter4/closest_point_to_triangle.html
//TODO: gamepad buttons getdown and getup functions
//TODO: shader standard library
//TODO: audio start
//TODO: variance shadow mapping, screen space soft shadows?
