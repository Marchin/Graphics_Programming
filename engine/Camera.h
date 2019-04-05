#ifndef CAMERA_H
#define CAMERA_H

// Default camera values
const f32 YAW = -90.0f;
const f32 PITCH = 0.0f;
const f32 SPEED = 2.5f;
const f32 SENSITIVITY = 0.1f;
const f32 ZOOM = 45.0f;

enum Projection {
    ORTHOGRAPHIC,
    PERSPECTIVE
};

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
struct ENGINE_API Camera {
	// Camera Attributes
	hmm_vec3 position;
	hmm_vec3 front;
	hmm_vec3 up;
	hmm_vec3 right;
	hmm_vec3 worldUp;
    hmm_mat4 model;
    hmm_mat4 view;
    hmm_mat4 projection;
    
	// Euler Angles
	f32 yaw;
	f32 pitch;
	// Camera options
	f32 movementSpeed;
	f32 mouseSensitivity;
	f32 zoom;
    
    f32 halfCamWidth;
	f32 halfCamHeight;
    f32 fov;
    f32 aspectRatio;
    Projection projectionType;
};

// Constructor with vectors
ENGINE_API void initCamera(Camera* pCamera,
                           hmm_vec3 position = HMM_Vec3(0.0f, 0.0f, 0.0f), 
                           hmm_vec3 up = HMM_Vec3(0.0f, 1.0f, 0.0f),
                           f32 yaw = YAW, f32 pitch = PITCH);

// Constructor with scalar values
ENGINE_API void initCamera(Camera* pCamera, 
                           f32 posX, f32 posY, f32 posZ, 
                           f32 upX, f32 upY, f32 upZ, 
                           f32 yaw, f32 pitch);
// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
ENGINE_API  hmm_mat4 cameraGetViewMatrix(Camera* pCamera);

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
ENGINE_API void  cameraProcessKeyboard(Camera* pCamera, hmm_vec3 direction, f32 deltaTime);

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
ENGINE_API void cameraProcessMouseMovement(Camera* pCamera, 
                                           f32 xoffset, f32 yoffset, 
                                           b32 constrainPitch = true);

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
ENGINE_API void cameraProcessMouseScroll(Camera* pCamera, f32 yoffset);

// Calculates the front vector from the Camera's (updated) Euler Angles
ENGINE_API void updateCameraVectors(Camera* pCamera);
#endif //CAMERA_H