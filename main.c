#include "src/DGNEngine/DGNEngine.h"

#define ASSERT_RETURN(x) if(!(x)) return 1

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 680

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

    DgnCamera camera;
    camera.far_plane    = 100;
    camera.near_plane   = 0.1;
    camera.fov          = 90 * TO_RADS;
    camera.height       = WINDOW_HEIGHT;
    camera.width        = WINDOW_WIDTH;
    camera.pos          = (Vec3){0.0f, 1.0f, 0.0f};
    camera.rot          = (Quat){0.0f, 0.0f, 0.0f, 1.0f};

    /** -------- GAME SETUP -------- **/

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

    DgnShader *skybox_shader = NULL;
    DgnShader *lit_shader = NULL;

    ASSERT_RETURN(level_mesh = dgnMeshLoad("res/game/test_level_1.obj", &level_mesh_count));

    ASSERT_RETURN(skybox_texture = dgnCubemapLoad(skybox_locations, DGN_TEX_WRAP_REPEAT, DGN_TEX_FILTER_TRILINEAR, DGN_TEX_STORAGE_SRGB));

    ASSERT_RETURN(skybox_shader = dgnShaderLoad("res/game/skybox.vert", 0, "res/game/skybox.frag"));
    ASSERT_RETURN(lit_shader = dgnShaderLoad("res/game/lit.vert", 0, "res/game/lit.frag"));

    int skybox_u_vp = dgnShaderGetUniformLoc(skybox_shader, "uVP");
    int skybox_u_sun_dir = dgnShaderGetUniformLoc(skybox_shader, "uSunDir");

    int lit_u_model = dgnShaderGetUniformLoc(lit_shader, "uModel");
    int lit_u_view = dgnShaderGetUniformLoc(lit_shader, "uView");
    int lit_u_projection = dgnShaderGetUniformLoc(lit_shader, "uProjection");


    while(!dgnWindowShouldClose(window))
    {
        dgnInputPollEvents();

        /** -------- UPDATE -------- **/

        Vec3 sun_dir = m3dVec3Normalized(m3dQuatRotateVec3(m3dQuatAngleAxis(dgnEngineGetSeconds() / 30.0f, (Vec3){0.0f, 1.0f, 0.0f}),
                                         (Vec3){-1.0f, -1.0f, 1.0f}));

        /** ---- Camera ---- **/

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

        /** -------- RENDER -------- **/

        dgnRendererEnableClearFlag(DGN_CLEAR_FLAG_COLOR | DGN_CLEAR_FLAG_DEPTH);
        dgnRendererClear();

        dgnRendererSetDepthTest(DGN_DEPTH_PASS_LESS);
        dgnRendererBindShader(lit_shader);

        dgnShaderUniformM4x4(lit_u_model, m3dMat4x4InitIdentity());
        dgnShaderUniformM4x4(lit_u_view, dgnCameraGetView(camera));
        dgnShaderUniformM4x4(lit_u_projection, dgnCameraGetProjection(camera));

        for(int i = 0; i < level_mesh_count; i++)
        {
            dgnRendererBindMesh(level_mesh[i]);
            dgnRendererDrawMesh();
        }

        dgnRendererSetDepthTest(DGN_DEPTH_PASS_LEQUAL);
        dgnRendererBindShader(skybox_shader);
        dgnRendererBindCubemap(skybox_texture, 0);

        dgnShaderUniformM4x4(skybox_u_vp, vp_mat);
        dgnShaderUniformV3(skybox_u_sun_dir, sun_dir);

        dgnRendererDrawSkybox();

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
