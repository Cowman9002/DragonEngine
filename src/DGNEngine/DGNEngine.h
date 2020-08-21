#ifndef DGN_ENGINE_H
#define DGN_ENGINE_H

#include <m3d/m3d.h>
#include <stdint.h>

#ifndef D_INTERNAL_H
typedef void DgnWindow;
typedef void DgnInput;
typedef void DgnMesh;
typedef void DgnShader;
typedef void DgnTexture;
typedef void DgnFramebuffer;
typedef void DgnLight;
#endif // D_INTERNAL_H

typedef struct
{
    DgnTexture *texture;
    DgnFramebuffer *framebuffer;

    Mat4x4 view_mat;
    Mat4x4 proj_mat;
}DgnShadowMap;

typedef struct
{
    Vec3 max;
    Vec3 min;
}DgnBoundingBox;

typedef struct
{
    Vec3 center;
    float radius;
}DgnBoundingSphere;

typedef struct
{
    float fov;
    float near, far;
    float width, height;
}DgnFrustum;

typedef struct
{
    Vec3 pos;
    Quat rot;

    DgnFrustum frustum;
}DgnCamera;

typedef struct
{
    uint8_t hit;
}DgnCollisionData;

/** ---------------- Engine Functions*/

void dgnEngineTerminate();
double dgnEngineGetSeconds();

/** ---------------- Input Functions*/

void dgnInputPollEvents();

uint8_t dgnInputGetKey(uint16_t key);
uint8_t dgnInputGetKeyDown(uint16_t key);
uint8_t dgnInputGetKeyUp(uint16_t key);

uint8_t dgnInputGetMouseButton(uint8_t button);
uint8_t dgnInputGetMouseButtonDown(uint8_t button);
uint8_t dgnInputGetMouseButtonUp(uint8_t button);

int32_t dgnInputGetMouseX();
int32_t dgnInputGetMouseY();
float dgnInputGetMouseXDelta();
float dgnInputGetMouseYDelta();

float dgnInputGetGamepadAxis(uint8_t gamepad, uint8_t axis, float deadzone);
uint8_t dgnInputGetGamepadButton(uint8_t gamepad, uint8_t button);

/** ---------------- Window Functions*/

uint8_t dgnWindowCreate(DgnWindow **out_window, uint16_t width, uint16_t height, const char* title);
void dgnWindowDestroy(DgnWindow *window);
void dgnWindowTerminate();

void dgnWindowMakeCurrent(DgnWindow *window);
uint8_t dgnWindowShouldClose(DgnWindow *window);
void dgnWindowSwapBuffers(DgnWindow *window);

uint16_t dgnWindowGetWidth(DgnWindow *window);
uint16_t dgnWindowGetHeight(DgnWindow *window);
const char* dgnWindowGetTitle(DgnWindow *window);
uint64_t dgnWindowGetFrameCount(DgnWindow *window);
double dgnWindowGetDelta(DgnWindow *window);

void dgnWindowSetRawCursorMode(DgnWindow *window, uint8_t enabled);
void dgnWindowSetCursorMode(DgnWindow *window, uint32_t cursor_mode);
void dgnWindowSetInput(DgnWindow *window, DgnInput *input);
void dgnWindowSetWidth(DgnWindow *window, uint16_t new_width);
void dgnWindowSetHeight(DgnWindow *window, uint16_t new_height);
void dgnWindowSetSize(DgnWindow *window, uint16_t new_width, uint16_t new_height);
void dgnWindowSetTitle(DgnWindow *window, const char* new_title);

/** ---------------- Rendering Functions*/

uint8_t dgnRendererInitialize();
void dgnRendererTerminate();

void dgnRendererClear();

void dgnRendererBindMesh(DgnMesh* mesh);
void dgnRendererBindShader(DgnShader* shader);
void dgnRendererBindTexture(DgnTexture *texture, uint8_t slot);
void dgnRendererBindCubemap(DgnTexture *texture, uint8_t slot);

void dgnRendererDrawMesh();
void dgnRendererDrawSkybox();
void dgnRendererDrawScreenTexture();

void dgnRendererSetDepthTest(uint16_t func);
void dgnRendererSetClearColor(float red, float green, float blue);
void dgnRendererSetVsync(uint8_t sync);
void dgnRendererSetDrawMode(uint8_t mode);
void dgnRendererSetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void dgnRendererSetCullFace(uint8_t face);
void dgnRendererSetWinding(uint8_t face);
void dgnRendererSetAlphaBlend(uint16_t sfactor, uint16_t dfactor);

void dgnRendererEnableClearFlag(unsigned int flag);
void dgnRendererDisableClearFlag(unsigned int flag);

void dgnRendererEnableFlag(uint16_t value);
void dgnRendererDisableFlag(uint16_t value);

void dgnRendererSetupShadow(DgnShadowMap shadow, DgnShader *shader, int32_t uniform_loc, Mat4x4 light_view_mat);

/** ---------------- Lighting Functions ---------------- **/

// dir must be normalized going in
Mat4x4 dgnLightingCreateDirViewMat(Vec3 dir);
Mat4x4 dgnLightingCreateLightSpaceMat(DgnShadowMap shadow);
DgnTexture *dgnLightingCreateShadowMap(uint16_t width, uint16_t height, uint8_t light_type);

Mat4x4 dgnLightingCreateLightProjMat(DgnCamera cam, DgnShadowMap shadow, DgnFrustum frustum, float near_pull);

/** ---------------- Collision Functions---------------- **/

DgnBoundingBox dgnCollisionGenerateBox(Vec3 *points, size_t points_count);
DgnBoundingSphere dgnCollisionGenerateSphere(Vec3 *points, size_t points_count);

DgnBoundingSphere dgnCollisionSphereFromBox(DgnBoundingBox box);

DgnCollisionData dgnCollisionBoxPoint(DgnBoundingBox box, Vec3 point);
DgnCollisionData dgnCollisionBoxBox(DgnBoundingBox a, DgnBoundingBox b);
DgnCollisionData dgnCollisionBoxSphere(DgnBoundingBox box, DgnBoundingSphere sphere);
DgnCollisionData dgnCollisionSphereSphere(DgnBoundingSphere a, DgnBoundingSphere b);
DgnCollisionData dgnCollisionSpherePoint(DgnBoundingSphere sphere, Vec3 point);

Vec3 dgnCollisionBoxGetCenter(DgnBoundingBox box);

/** ---------------- Camera Functions ---------------- **/

Mat4x4 dgnCameraGetProjection(DgnCamera cam);
Mat4x4 dgnCameraGetView(DgnCamera cam);
Mat4x4 dgnCameraGetInverseView(DgnCamera cam);

/** ---------------- Mesh Functions ---------------- **/

DgnMesh *dgnMeshCreate(
    float vertex_data[],            /** consists of all data for all vertices in the mesh*/
    size_t vertex_data_size,        /** size in bytes of the vertex_data array*/
    uint32_t index_data[],          /** ordered array for the index of each vertex to use in the vertex_data array*/
    size_t index_data_size,         /** size in bytes of the index_data array*/
    uint16_t mesh_type);            /** bit field of the types of data in vertex_data array*/

DgnMesh **dgnMeshLoad(const char *filepath, uint16_t *out_num_meshes);

void dgnMeshDestroy(DgnMesh *mesh);
void dgnMeshDestroyArr(DgnMesh **meshes, uint16_t num_meshes);

/** ---------------- Shader functions ---------------- **/

DgnShader *dgnShaderCreate(char* vertex_code, char* geometry_code, char* fragment_code);

DgnShader *dgnShaderLoad(const char* vertex_path, const char* geometry_path, const char* fragment_path);

void dgnShaderDestroy(DgnShader *shader);

int32_t dgnShaderGetUniformLoc(DgnShader *shader, const char *name);

void dgnShaderUniformF(int32_t loc, float value);
void dgnShaderUniformI(int32_t loc, int value);
void dgnShaderUniformB(int32_t loc, uint8_t value);
void dgnShaderUniformV3(int32_t loc, Vec3 value);
void dgnShaderUniformV2(int32_t loc, Vec2 value);
void dgnShaderUniformM3x3(int32_t loc, Mat3x3 value);
void dgnShaderUniformM4x4(int32_t loc, Mat4x4 value);

void dgnShaderSetEconstI(const char *name, int value);

/** ---------------- Texture Functions ---------------- **/

DgnTexture *dgnTextureCreate(
    uint8_t *data,              /** pixel data*/
    uint32_t width,             /** width in pixels of the texture*/
    uint32_t height,            /** height in pixels of the texture*/
    uint8_t wrapping,           /** texture wrap*/
    uint8_t filtering,          /** texture filtering*/
    uint8_t mipmapped,           /** generate mipmaps*/
    uint16_t storage_type,      /** how data is given to texture*/
    uint16_t internal_type,     /** how data is stored inside texture*/
    uint16_t data_type);        /** what kind of data is stored*/

DgnTexture *dgnCubemapCreate(
    uint8_t *data[6],      /** pixel data array of each face in order, +x, -x, +y, -y, +z, -z*/
    uint32_t *width,     /** array of each faces width in pixels in same order as data*/
    uint32_t *height,    /** array of each faces height in pixels in same order as data*/
    uint8_t wrapping,   /** texture wrap*/
    uint8_t filtering,
    uint16_t storage_type);  /** texture filtering*/

DgnTexture *dgnTextureLoad(const char *filepath, uint8_t wrapping, uint8_t filtering, uint8_t mipmapped, uint16_t storage_type);
DgnTexture *dgnCubemapLoad(const char *filepath[6], uint8_t wrapping, uint8_t filtering, uint16_t storage_type);

void dgnTextureDestroy(DgnTexture *texture);

void dgnTextureSetWrap(DgnTexture *texture, uint8_t wrap_mode);
void dgnTextureSetFilter(DgnTexture *texture, uint8_t filter_mode);
void dgnTextureSetBorderColor(DgnTexture *texture, float r, float g, float b, float a);

uint32_t dgnTextureGetWidth(DgnTexture *texture);
uint32_t dgnTextureGetHeight(DgnTexture *texture);

/** ---------------- FrameBuffer Functions ---------------- **/

DgnFramebuffer *dgnFramebufferCreate(DgnTexture **dst_textures, uint8_t *attachment_types, uint8_t num_textures, uint8_t flags);
void dgnFramebufferDestroy(DgnFramebuffer *buffer);

void dgnFramebufferBind(DgnFramebuffer *buffer);

#define DGN_TRUE 1
#define DGN_FALSE 0

#define DGN_VSYNC_OFF 0
#define DGN_VSYNC_SINGLE 1
#define DGN_VSYNC_DOUBLE 2

#define DGN_CURSOR_NORMAL 0x00034001
#define DGN_CURSOR_HIDDEN 0x00034002
#define DGN_CURSOR_DISABLED 0x00034003

#define DGN_CLEAR_FLAG_COLOR 0x00004000
#define DGN_CLEAR_FLAG_DEPTH 0x00000100
#define DGN_CLEAR_FLAG_STENCIL 0x00000400

#define DGN_RENDER_FLAG_ALPHA_BLEND 0x0BE2
#define DGN_RENDER_FLAG_CULL_FACE 0x0B44
#define DGN_RENDER_FLAG_DEPTH_TEST 0x0B71
#define DGN_RENDER_FLAG_MULTISAMPLING 0x809D
#define DGN_RENDER_FLAG_SCISSOR_TEST 0x0C11
#define DGN_RENDER_FLAG_STENCIL_TEST 0x0B90
#define DGN_RENDER_FLAG_SEAMLESS_CUBEMAP 0x884F

#define DGN_DEPTH_PASS_NEVER 0x0200
#define DGN_DEPTH_PASS_LESS 0x0201
#define DGN_DEPTH_PASS_EQUAL 0x0202
#define DGN_DEPTH_PASS_LEQUAL 0x0203
#define DGN_DEPTH_PASS_GREATER 0x0204
#define DGN_DEPTH_PASS_NOTEQUAL 0x0205
#define DGN_DEPTH_PASS_GEQUAL 0x0206
#define DGN_DEPTH_PASS_ALWAYS 0x0207

#define DGN_ALPHA_BLEND_ZERO 0                          //Factor is equal to 0
#define DGN_ALPHA_BLEND_ONE 1                           //equal to 1
#define DGN_ALPHA_BLEND_SRC_COLOR 0x0300                //equal to the source color vector
#define DGN_ALPHA_BLEND_ONE_MINUS_SRC_COLOR 0x0301      //equal to 1 minus the source color vector
#define DGN_ALPHA_BLEND_DST_COLOR 0x0306                //equal to the destination color vector
#define DGN_ALPHA_BLEND_ONE_MINUS_DST_COLOR 0x0307      //equal to 1 minus the destination color vector
#define DGN_ALPHA_BLEND_SRC_ALPHA 0x0302                //equal to the alpha component of the source color vector
#define DGN_ALPHA_BLEND_ONE_MINUS_SRC_ALPHA 0x0303      //equal to 1 − alpha of the source color vector
#define DGN_ALPHA_BLEND_DST_ALPHA 0x0304                //equal to the alpha component of the destination color vector
#define DGN_ALPHA_BLEND_ONE_MINUS_DST_ALPHA 0x0305      //equal to 1 - alpha component of the destination color vector
#define DGN_ALPHA_BLEND_CONSTANT_COLOR 0x8001           //equal to the constant color vector
#define DGN_ALPHA_BLEND_ONE_MINUS_CONSTANT_COLOR 0x8002 //equal to 1 - constant color vector
#define DGN_ALPHA_BLEND_CONSTANT_ALPHA 0x8003           //equal to the constant alpha vector
#define DGN_ALPHA_BLEND_ONE_MINUS_CONSTANT_ALPHA 0x8004 //equal to 1 - constant alpha vector

#define DGN_FACE_CLOCKWISE 0
#define DGN_FACE_CCLOCKWISE 1

#define DGN_FACE_FRONT 0
#define DGN_FACE_BACK 1
#define DGN_FACE_FRONT_BACK 4

#define DGN_DRAW_MODE_POINTS 0x00
#define DGN_DRAW_MODE_LINES 0x01
#define DGN_DRAW_MODE_LINE_LOOP 0x02
#define DGN_DRAW_MODE_LINE_STRIP 0x03
#define DGN_DRAW_MODE_TRIANGLES 0x04
#define DGN_DRAW_MODE_TRIANGLE_STRIP 0x05
#define DGN_DRAW_MODE_TRIANGLE_FAN 0x06

#define DGN_VERT_ATTRIB_POSITION 0x0001
#define DGN_VERT_ATTRIB_TEXCOORD 0x0002
#define DGN_VERT_ATTRIB_NORMAL 0x0004
#define DGN_VERT_ATTRIB_COLOR 0x0008
#define DGN_VERT_ATTRIB_TANGENT 0x0010
// When something is added here, change NUM_MESH_TYPE_INTERNAL

#define DGN_TEX_WRAP_REPEAT 0X00
#define DGN_TEX_WRAP_MIRROR 0X01
#define DGN_TEX_WRAP_CLAMP_TO_EDGE 0x02
#define DGN_TEX_WRAP_CLAMP_TO_BOARDER 0x03

#define DGN_TEX_FILTER_NEAREST 0x00
#define DGN_TEX_FILTER_BILINEAR 0x01
#define DGN_TEX_FILTER_TRILINEAR 0x02

#define DGN_TEX_STORAGE_RGB 0x1907
#define DGN_TEX_STORAGE_RGBA 0x1908
#define DGN_TEX_STORAGE_RGB16F 0x881B
#define DGN_TEX_STORAGE_RGBA16F 0x881A
#define DGN_TEX_STORAGE_RGB32F 0x8815
#define DGN_TEX_STORAGE_RGBA32F 0x8814
#define DGN_TEX_STORAGE_SRGB 0x8C40
#define DGN_TEX_STORAGE_SRGBA 0x8C42
#define DGN_TEX_STORAGE_DEPTH 0x1902

#define DGN_DATA_TYPE_UBYTE 0x1401
#define DGN_DATA_TYPE_FLOAT 0x1406

#define DGN_FRAMEBUFFER_DEPTH 0x01
#define DGN_FRAMEBUFFER_COLOR 0X02

#define DGN_LIGHT_TYPE_DIR 0X00
#define DGN_LIGHT_TYPE_POINT 0X01
#define DGN_LIGHT_TYPE_SPOT 0X02

/* The unknown key */
#define DGN_KEY_UNKNOWN            -1

/* Printable keys */
#define DGN_KEY_SPACE              32
#define DGN_KEY_APOSTROPHE         39  /* ' */
#define DGN_KEY_COMMA              44  /* , */
#define DGN_KEY_MINUS              45  /* - */
#define DGN_KEY_PERIOD             46  /* . */
#define DGN_KEY_SLASH              47  /* / */
#define DGN_KEY_0                  48
#define DGN_KEY_1                  49
#define DGN_KEY_2                  50
#define DGN_KEY_3                  51
#define DGN_KEY_4                  52
#define DGN_KEY_5                  53
#define DGN_KEY_6                  54
#define DGN_KEY_7                  55
#define DGN_KEY_8                  56
#define DGN_KEY_9                  57
#define DGN_KEY_SEMICOLON          59  /* ; */
#define DGN_KEY_EQUAL              61  /* = */
#define DGN_KEY_A                  65
#define DGN_KEY_B                  66
#define DGN_KEY_C                  67
#define DGN_KEY_D                  68
#define DGN_KEY_E                  69
#define DGN_KEY_F                  70
#define DGN_KEY_G                  71
#define DGN_KEY_H                  72
#define DGN_KEY_I                  73
#define DGN_KEY_J                  74
#define DGN_KEY_K                  75
#define DGN_KEY_L                  76
#define DGN_KEY_M                  77
#define DGN_KEY_N                  78
#define DGN_KEY_O                  79
#define DGN_KEY_P                  80
#define DGN_KEY_Q                  81
#define DGN_KEY_R                  82
#define DGN_KEY_S                  83
#define DGN_KEY_T                  84
#define DGN_KEY_U                  85
#define DGN_KEY_V                  86
#define DGN_KEY_W                  87
#define DGN_KEY_X                  88
#define DGN_KEY_Y                  89
#define DGN_KEY_Z                  90
#define DGN_KEY_LEFT_BRACKET       91  /* [ */
#define DGN_KEY_BACKSLASH          92  /* \ */
#define DGN_KEY_RIGHT_BRACKET      93  /* ] */
#define DGN_KEY_GRAVE_ACCENT       96  /* ` */
#define DGN_KEY_WORLD_1            161 /* non-US #1 */
#define DGN_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define DGN_KEY_ESCAPE             256
#define DGN_KEY_ENTER              257
#define DGN_KEY_TAB                258
#define DGN_KEY_BACKSPACE          259
#define DGN_KEY_INSERT             260
#define DGN_KEY_DELETE             261
#define DGN_KEY_RIGHT              262
#define DGN_KEY_LEFT               263
#define DGN_KEY_DOWN               264
#define DGN_KEY_UP                 265
#define DGN_KEY_PAGE_UP            266
#define DGN_KEY_PAGE_DOWN          267
#define DGN_KEY_HOME               268
#define DGN_KEY_END                269
#define DGN_KEY_CAPS_LOCK          280
#define DGN_KEY_SCROLL_LOCK        281
#define DGN_KEY_NUM_LOCK           282
#define DGN_KEY_PRINT_SCREEN       283
#define DGN_KEY_PAUSE              284
#define DGN_KEY_F1                 290
#define DGN_KEY_F2                 291
#define DGN_KEY_F3                 292
#define DGN_KEY_F4                 293
#define DGN_KEY_F5                 294
#define DGN_KEY_F6                 295
#define DGN_KEY_F7                 296
#define DGN_KEY_F8                 297
#define DGN_KEY_F9                 298
#define DGN_KEY_F10                299
#define DGN_KEY_F11                300
#define DGN_KEY_F12                301
#define DGN_KEY_F13                302
#define DGN_KEY_F14                303
#define DGN_KEY_F15                304
#define DGN_KEY_F16                305
#define DGN_KEY_F17                306
#define DGN_KEY_F18                307
#define DGN_KEY_F19                308
#define DGN_KEY_F20                309
#define DGN_KEY_F21                310
#define DGN_KEY_F22                311
#define DGN_KEY_F23                312
#define DGN_KEY_F24                313
#define DGN_KEY_F25                314
#define DGN_KEY_KP_0               320
#define DGN_KEY_KP_1               321
#define DGN_KEY_KP_2               322
#define DGN_KEY_KP_3               323
#define DGN_KEY_KP_4               324
#define DGN_KEY_KP_5               325
#define DGN_KEY_KP_6               326
#define DGN_KEY_KP_7               327
#define DGN_KEY_KP_8               328
#define DGN_KEY_KP_9               329
#define DGN_KEY_KP_DECIMAL         330
#define DGN_KEY_KP_DIVIDE          331
#define DGN_KEY_KP_MULTIPLY        332
#define DGN_KEY_KP_SUBTRACT        333
#define DGN_KEY_KP_ADD             334
#define DGN_KEY_KP_ENTER           335
#define DGN_KEY_KP_EQUAL           336
#define DGN_KEY_LEFT_SHIFT         340
#define DGN_KEY_LEFT_CONTROL       341
#define DGN_KEY_LEFT_ALT           342
#define DGN_KEY_LEFT_SUPER         343
#define DGN_KEY_RIGHT_SHIFT        344
#define DGN_KEY_RIGHT_CONTROL      345
#define DGN_KEY_RIGHT_ALT          346
#define DGN_KEY_RIGHT_SUPER        347
#define DGN_KEY_MENU               348

#define DGN_KEY_LAST               DGN_KEY_MENU

#define DGN_MOUSE_BUTTON_1         0
#define DGN_MOUSE_BUTTON_2         1
#define DGN_MOUSE_BUTTON_3         2
#define DGN_MOUSE_BUTTON_4         3
#define DGN_MOUSE_BUTTON_5         4
#define DGN_MOUSE_BUTTON_6         5
#define DGN_MOUSE_BUTTON_7         6
#define DGN_MOUSE_BUTTON_8         7
#define DGN_MOUSE_BUTTON_LAST      DGN_MOUSE_BUTTON_8
#define DGN_MOUSE_BUTTON_LEFT      DGN_MOUSE_BUTTON_1
#define DGN_MOUSE_BUTTON_RIGHT     DGN_MOUSE_BUTTON_2
#define DGN_MOUSE_BUTTON_MIDDLE    DGN_MOUSE_BUTTON_3

#define DGN_GAMEPAD_1       0
#define DGN_GAMEPAD_2       1
#define DGN_GAMEPAD_3       2
#define DGN_GAMEPAD_4       3
#define DGN_GAMEPAD_5       4
#define DGN_GAMEPAD_6       5
#define DGN_GAMEPAD_7       6
#define DGN_GAMEPAD_8       7
#define DGN_GAMEPAD_9       8
#define DGN_GAMEPAD_10      9
#define DGN_GAMEPAD_11      10
#define DGN_GAMEPAD_12      11
#define DGN_GAMEPAD_13      12
#define DGN_GAMEPAD_14      13
#define DGN_GAMEPAD_15      14
#define DGN_GAMEPAD_16      15
#define DGN_GAMEPAD_LAST    DGN_GAMEPAD_16

#define DGN_GAMEPAD_BUTTON_A               0
#define DGN_GAMEPAD_BUTTON_B               1
#define DGN_GAMEPAD_BUTTON_X               2
#define DGN_GAMEPAD_BUTTON_Y               3
#define DGN_GAMEPAD_BUTTON_LEFT_BUMPER     4
#define DGN_GAMEPAD_BUTTON_RIGHT_BUMPER    5
#define DGN_GAMEPAD_BUTTON_BACK            6
#define DGN_GAMEPAD_BUTTON_START           7
#define DGN_GAMEPAD_BUTTON_GUIDE           8
#define DGN_GAMEPAD_BUTTON_LEFT_THUMB      9
#define DGN_GAMEPAD_BUTTON_RIGHT_THUMB     10
#define DGN_GAMEPAD_BUTTON_DPAD_UP         11
#define DGN_GAMEPAD_BUTTON_DPAD_RIGHT      12
#define DGN_GAMEPAD_BUTTON_DPAD_DOWN       13
#define DGN_GAMEPAD_BUTTON_DPAD_LEFT       14
#define DGN_GAMEPAD_BUTTON_LAST            DGN_GAMEPAD_BUTTON_DPAD_LEFT

#define DGN_GAMEPAD_BUTTON_CROSS       DGN_GAMEPAD_BUTTON_A
#define DGN_GAMEPAD_BUTTON_CIRCLE      DGN_GAMEPAD_BUTTON_B
#define DGN_GAMEPAD_BUTTON_SQUARE      DGN_GAMEPAD_BUTTON_X
#define DGN_GAMEPAD_BUTTON_TRIANGLE    DGN_GAMEPAD_BUTTON_Y

#define DGN_GAMEPAD_AXIS_LEFT_X        0
#define DGN_GAMEPAD_AXIS_LEFT_Y        1
#define DGN_GAMEPAD_AXIS_RIGHT_X       2
#define DGN_GAMEPAD_AXIS_RIGHT_Y       3
#define DGN_GAMEPAD_AXIS_LEFT_TRIGGER  4
#define DGN_GAMEPAD_AXIS_RIGHT_TRIGGER 5
#define DGN_GAMEPAD_AXIS_LAST          DGN_GAMEPAD_AXIS_RIGHT_TRIGGER

#endif // DGN_ENGINE_H
