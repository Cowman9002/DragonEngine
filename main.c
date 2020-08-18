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
#define SHADOW_FAR 40.0f
#define SHADOW_NEAR 0.1f
#define CASCADE_SPLIT_BLEND 0.45f

#include "src/c_ordered_map.h"
#include "src/c_linked_list.h"

#include <MemLeaker/malloc.h>

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
    camera.far_plane    = 100;
    camera.near_plane   = 0.1;
    camera.fov          = 90 * TO_RADS;
    camera.height       = WINDOW_HEIGHT;
    camera.width        = WINDOW_WIDTH;
    camera.pos          = (Vec3){0.0f, 1.0f, 0.0f};
    camera.rot          = (Quat){0.0f, 0.0f, 0.0f, 1.0f};

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
                                                  DGN_TEX_FILTER_NEAREST, DGN_FALSE, DGN_TEX_STORAGE_RGBA, DGN_TEX_STORAGE_RGBA16F,
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

    DgnTexture *skybox_texture = NULL;
    DgnTexture *checker_textures[4] = {NULL, NULL, NULL, NULL};

    DgnShader *skybox_shader = NULL;
    DgnShader *lit_shader = NULL;
    DgnShader *screen_shader = NULL;
    DgnShader *shadow_shader = NULL;

    dgnShaderSetEconstI("NUM_CASCADES", CASCADE_COUNT);

    ASSERT_RETURN(level_mesh = dgnMeshLoad("res/game/test_level_1.obj", &level_mesh_count));

    ASSERT_RETURN(skybox_texture = dgnCubemapLoad(skybox_locations, DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(checker_textures[0] = dgnTextureLoad("res/game/checker1.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(checker_textures[1] = dgnTextureLoad("res/game/checker2.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(checker_textures[2] = dgnTextureLoad("res/game/checker3.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));
    ASSERT_RETURN(checker_textures[3] = dgnTextureLoad("res/game/checker4.png", DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TRUE, DGN_TEX_STORAGE_SRGB));

    ASSERT_RETURN(skybox_shader = dgnShaderLoad("res/game/skybox.vert", 0, "res/game/skybox.frag"));
    ASSERT_RETURN(lit_shader = dgnShaderLoad("res/game/lit.vert", 0, "res/game/lit.frag"));
    ASSERT_RETURN(screen_shader = dgnShaderLoad("res/game/screen.vert", 0, "res/game/screen.frag"));
    ASSERT_RETURN(shadow_shader = dgnShaderLoad("res/game/shadow.vert", 0, 0));

    int skybox_u_vp = dgnShaderGetUniformLoc(skybox_shader, "uVP");
    int skybox_u_sun_dir = dgnShaderGetUniformLoc(skybox_shader, "uSunDir");

    int lit_u_model = dgnShaderGetUniformLoc(lit_shader, "uModel");
    int lit_u_view = dgnShaderGetUniformLoc(lit_shader, "uView");
    int lit_u_projection = dgnShaderGetUniformLoc(lit_shader, "uProjection");
    int lit_u_texture = dgnShaderGetUniformLoc(lit_shader, "uTexture");
    int lit_u_has_texture = dgnShaderGetUniformLoc(lit_shader, "uHasTexture");
    int lit_u_skybox = dgnShaderGetUniformLoc(lit_shader, "uSkybox");
    int lit_u_light_dir = dgnShaderGetUniformLoc(lit_shader, "uLightDir");
    int lit_u_cam_pos = dgnShaderGetUniformLoc(lit_shader, "uCamPos");

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


    while(!dgnWindowShouldClose(window))
    {
        dgnInputPollEvents();

        /** ---------------- UPDATE ---------------- **/

        Vec3 sun_dir = m3dVec3Normalized(m3dQuatRotateVec3(m3dQuatAngleAxis(dgnEngineGetSeconds() / 30.0f, (Vec3){0.0f, 1.0f, 0.0f}),
                                         (Vec3){-1.0f, -1.0f, -1.0f}));

        /** -------- Camera -------- **/

        float camera_rot_speed = PI * dgnWindowGetDelta(window);
        float camera_move_speed = 3.0f * dgnWindowGetDelta(window);
        Quat camera_rot_y;
        Quat camera_rot_x;

        camera_rot_y = m3dQuatAngleAxis(
                        dgnInputGetGamepadAxis(DGN_GAMEPAD_1, DGN_GAMEPAD_AXIS_RIGHT_X, 0.1f) * camera_rot_speed,
                        (Vec3){0.0f, -1.0f, 0.0f});

        camera_rot_x = m3dQuatAngleAxis(
                        dgnInputGetGamepadAxis(DGN_GAMEPAD_1, DGN_GAMEPAD_AXIS_RIGHT_Y, 0.1f) * camera_rot_speed,
                        m3dQuatRotateVec3(camera.rot, (Vec3){-1.0f, 0.0f, 0.0f}));

        camera.rot = m3dQuatMulQuat(m3dQuatMulQuat(camera_rot_y, camera_rot_x), camera.rot);

        camera.pos = m3dVec3AddVec3(camera.pos,
                                    m3dQuatRotateVec3(camera.rot, (Vec3){0.0f, 0.0f,
                                    camera_move_speed * dgnInputGetGamepadAxis(DGN_GAMEPAD_1, DGN_GAMEPAD_AXIS_LEFT_Y, 0.1f)}));

        camera.pos = m3dVec3AddVec3(camera.pos,
                                    m3dQuatRotateVec3(camera.rot, (Vec3){
                                    camera_move_speed * dgnInputGetGamepadAxis(DGN_GAMEPAD_1, DGN_GAMEPAD_AXIS_LEFT_X, 0.1f),
                                    0.0f, 0.0f}));

        Mat4x4 vp_mat = m3dMat4x4MulMat4x4(dgnCameraGetProjection(camera),
                        m3dMat4x4FromMat3x3(m3dMat3x3FromMat4x4(dgnCameraGetView(camera))));
        //Mat4x4 vp_mat = dgnCameraGetProjection(camera);

        /** -------- Shadows -------- **/

        for(int i = 0; i < CASCADE_COUNT; i++)
        {
            shadow_cascades[i].view_mat = dgnLightingCreateDirViewMat(sun_dir);
            shadow_cascades[i].proj_mat = dgnLightingCreateLightProjMat(
                                            dgnCameraGetInverseView(camera),
                                            shadow_cascades[i].view_mat,
                                            WINDOW_WIDTH, WINDOW_HEIGHT, 90 * TO_RADS,
                                            cascade_depths[i], cascade_depths[i + 1], 15.0f);
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

        dgnShaderUniformM4x4(lit_u_model, m3dMat4x4InitIdentity());
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

        dgnShaderUniformB(lit_u_has_texture, DGN_TRUE);
        dgnShaderUniformI(lit_u_texture, 0);
        for(int i = 0; i < level_mesh_count; i++)
        {
            if(i < 4)
            {
                dgnRendererBindTexture(checker_textures[i], 0);
            }
            dgnRendererBindMesh(level_mesh[i]);
            dgnRendererDrawMesh();
        }

        dgnRendererSetDepthTest(DGN_DEPTH_PASS_LEQUAL);
        dgnRendererBindShader(skybox_shader);
        dgnRendererBindCubemap(skybox_texture, 0);

        dgnShaderUniformM4x4(skybox_u_vp, vp_mat);
        dgnShaderUniformV3(skybox_u_sun_dir, sun_dir);

        dgnRendererDrawSkybox();

        dgnFramebufferBind(0);

        dgnRendererSetDepthTest(DGN_DEPTH_PASS_ALWAYS);
        dgnRendererBindShader(screen_shader);

        dgnShaderUniformB(screen_u_single, DGN_FALSE);
        dgnShaderUniformV2(screen_u_scale, (Vec2){1.0f, 1.0f});
        dgnShaderUniformV2(screen_u_offset, (Vec2){0.0f, 0.0f});

        dgnRendererBindTexture(screen_texture, 0);

        dgnRendererDrawScreenTexture();

        for(int i = 0; i < CASCADE_COUNT; i++)
        {
            float cc_inverse = 1.0f / 10;

            dgnShaderUniformB(screen_u_single, DGN_TRUE);
            dgnShaderUniformV2(screen_u_scale, (Vec2){cc_inverse, cc_inverse * (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT});

            dgnShaderUniformV2(screen_u_offset, (Vec2){ i * 2.0f * cc_inverse, 2.0f - 2.0f * cc_inverse * (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT});

            dgnRendererBindTexture(shadow_cascades[i].texture, 0);

            dgnRendererDrawScreenTexture();
        }

        dgnWindowSwapBuffers(window);
    }

    dgnMeshDestroyArr(level_mesh, level_mesh_count);

    dgnTextureDestroy(skybox_texture);

    dgnShaderDestroy(skybox_shader);
    dgnShaderDestroy(lit_shader);

    dgnRendererTerminate();
    dgnWindowDestroy(window);
    dgnEngineTerminate();
}

// collision start
// audio start
