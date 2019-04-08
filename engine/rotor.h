#ifndef ROTOR_H
#define ROTOR_H

//http://marctenbosch.com/quaternions/

struct Bivec3 {
    float xy;
    float xz;
    float yz;
};

struct Rotor3 {
    float a;
    float xy;
    float xz;
    float yz;
};

global const Bivec3 BivecXY = Bivec3{1, 0, 0};
global const Bivec3 BivecXZ = Bivec3{0, 1, 0};
global const Bivec3 BivecYZ = Bivec3{0, 0, 1};

global const Rotor3 RotorXY = Rotor3{0, 1, 0, 0};
global const Rotor3 RotorXZ = Rotor3{0, 0, 1, 0};
global const Rotor3 RotorYZ = Rotor3{0, 0, 0, 1};

ENGINE_API inline Bivec3 outer(const hmm_vec3* pVec3A, const hmm_vec3* pVec3B);
ENGINE_API Rotor3 rotorFromVecAToVecB(const hmm_vec3* pVec3A, const hmm_vec3* pVec3B);
ENGINE_API Rotor3 rotorFromAngleAndBivec(float angleRadian, Bivec3 bivec);
ENGINE_API inline Rotor3 operator*(Rotor3 rotA, Rotor3 rotB);
ENGINE_API hmm_vec3 getRotatedVector(hmm_vec3 vec, Rotor3 rot);
ENGINE_API inline Rotor3 &operator*=(Rotor3& rotA, Rotor3 rotB);
ENGINE_API inline Rotor3 getRotatedRotor(Rotor3 rotated, Rotor3 rotor);
ENGINE_API inline Rotor3 rotorGetReverse(Rotor3 rot);
ENGINE_API inline f32 rotorLengthSqrd(Rotor3* pRot);
ENGINE_API inline f32 rotorLength(Rotor3* pRot);
ENGINE_API inline void rotorNormalize(Rotor3* pRot);

#endif //ROTOR_H