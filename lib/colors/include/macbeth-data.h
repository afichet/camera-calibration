#ifndef MACBETH_DATA_H_
#define MACBETH_DATA_H_

const int macbeth_wavelengths[36]
    = {380, 390, 400, 410, 420, 430, 440, 450, 460, 470, 480, 490, 500, 510, 520, 530, 540, 550,
       560, 570, 580, 590, 600, 610, 620, 630, 640, 650, 660, 670, 680, 690, 700, 710, 720, 730};
const float macbeth_patches[24][36]
    = {{0.055f, 0.058f, 0.061f, 0.062f, 0.062f, 0.062f, 0.062f, 0.062f, 0.062f, 0.062f, 0.062f, 0.063f,
        0.065f, 0.070f, 0.076f, 0.079f, 0.081f, 0.084f, 0.091f, 0.103f, 0.119f, 0.134f, 0.143f, 0.147f,
        0.151f, 0.158f, 0.168f, 0.179f, 0.188f, 0.190f, 0.186f, 0.181f, 0.182f, 0.187f, 0.196f, 0.209},
       {0.117f, 0.143f, 0.175f, 0.191f, 0.196f, 0.199f, 0.204f, 0.213f, 0.228f, 0.251f, 0.280f, 0.309f,
        0.329f, 0.333f, 0.315f, 0.286f, 0.273f, 0.276f, 0.277f, 0.289f, 0.339f, 0.420f, 0.488f, 0.525f,
        0.546f, 0.562f, 0.578f, 0.595f, 0.612f, 0.625f, 0.638f, 0.656f, 0.678f, 0.700f, 0.717f, 0.734},
       {0.130f, 0.177f, 0.251f, 0.306f, 0.324f, 0.330f, 0.333f, 0.331f, 0.323f, 0.311f, 0.298f, 0.285f,
        0.269f, 0.250f, 0.231f, 0.214f, 0.199f, 0.185f, 0.169f, 0.157f, 0.149f, 0.145f, 0.142f, 0.141f,
        0.141f, 0.141f, 0.143f, 0.147f, 0.152f, 0.154f, 0.150f, 0.144f, 0.136f, 0.132f, 0.135f, 0.147},
       {0.051f, 0.054f, 0.056f, 0.057f, 0.058f, 0.059f, 0.060f, 0.061f, 0.062f, 0.063f, 0.065f, 0.067f,
        0.075f, 0.101f, 0.145f, 0.178f, 0.184f, 0.170f, 0.149f, 0.133f, 0.122f, 0.115f, 0.109f, 0.105f,
        0.104f, 0.106f, 0.109f, 0.112f, 0.114f, 0.114f, 0.112f, 0.112f, 0.115f, 0.120f, 0.125f, 0.130},
       {0.144f, 0.198f, 0.294f, 0.375f, 0.408f, 0.421f, 0.426f, 0.426f, 0.419f, 0.403f, 0.379f, 0.346f,
        0.311f, 0.281f, 0.254f, 0.229f, 0.214f, 0.208f, 0.202f, 0.194f, 0.193f, 0.200f, 0.214f, 0.230f,
        0.241f, 0.254f, 0.279f, 0.313f, 0.348f, 0.366f, 0.366f, 0.359f, 0.358f, 0.365f, 0.377f, 0.398},
       {0.136f, 0.179f, 0.247f, 0.297f, 0.320f, 0.337f, 0.355f, 0.381f, 0.419f, 0.466f, 0.510f, 0.546f,
        0.567f, 0.574f, 0.569f, 0.551f, 0.524f, 0.488f, 0.445f, 0.400f, 0.350f, 0.299f, 0.252f, 0.221f,
        0.204f, 0.196f, 0.191f, 0.188f, 0.191f, 0.199f, 0.212f, 0.223f, 0.232f, 0.233f, 0.229f, 0.229},
       {0.054f, 0.054f, 0.053f, 0.054f, 0.054f, 0.055f, 0.055f, 0.055f, 0.056f, 0.057f, 0.058f, 0.061f,
        0.068f, 0.089f, 0.125f, 0.154f, 0.174f, 0.199f, 0.248f, 0.335f, 0.444f, 0.538f, 0.587f, 0.595f,
        0.591f, 0.587f, 0.584f, 0.584f, 0.590f, 0.603f, 0.620f, 0.639f, 0.655f, 0.663f, 0.663f, 0.667},
       {0.122f, 0.164f, 0.229f, 0.286f, 0.327f, 0.361f, 0.388f, 0.400f, 0.392f, 0.362f, 0.316f, 0.260f,
        0.209f, 0.168f, 0.138f, 0.117f, 0.104f, 0.096f, 0.090f, 0.086f, 0.084f, 0.084f, 0.084f, 0.084f,
        0.084f, 0.085f, 0.090f, 0.098f, 0.109f, 0.123f, 0.143f, 0.169f, 0.205f, 0.244f, 0.287f, 0.332},
       {0.096f, 0.115f, 0.131f, 0.135f, 0.133f, 0.132f, 0.130f, 0.128f, 0.125f, 0.120f, 0.115f, 0.110f,
        0.105f, 0.100f, 0.095f, 0.093f, 0.092f, 0.093f, 0.096f, 0.108f, 0.156f, 0.265f, 0.399f, 0.500f,
        0.556f, 0.579f, 0.588f, 0.591f, 0.593f, 0.594f, 0.598f, 0.602f, 0.607f, 0.609f, 0.609f, 0.610},
       {0.092f, 0.116f, 0.146f, 0.169f, 0.178f, 0.173f, 0.158f, 0.139f, 0.119f, 0.101f, 0.087f, 0.075f,
        0.066f, 0.060f, 0.056f, 0.053f, 0.051f, 0.051f, 0.052f, 0.052f, 0.051f, 0.052f, 0.058f, 0.073f,
        0.096f, 0.119f, 0.141f, 0.166f, 0.194f, 0.227f, 0.265f, 0.309f, 0.355f, 0.396f, 0.436f, 0.478},
       {0.061f, 0.061f, 0.062f, 0.063f, 0.064f, 0.066f, 0.069f, 0.075f, 0.085f, 0.105f, 0.139f, 0.192f,
        0.271f, 0.376f, 0.476f, 0.531f, 0.549f, 0.546f, 0.528f, 0.504f, 0.471f, 0.428f, 0.381f, 0.347f,
        0.327f, 0.318f, 0.312f, 0.310f, 0.314f, 0.327f, 0.345f, 0.363f, 0.376f, 0.381f, 0.378f, 0.379},
       {0.063f, 0.063f, 0.063f, 0.064f, 0.064f, 0.064f, 0.065f, 0.066f, 0.067f, 0.068f, 0.071f, 0.076f,
        0.087f, 0.125f, 0.206f, 0.305f, 0.383f, 0.431f, 0.469f, 0.518f, 0.568f, 0.607f, 0.628f, 0.637f,
        0.640f, 0.642f, 0.645f, 0.648f, 0.651f, 0.653f, 0.657f, 0.664f, 0.673f, 0.680f, 0.684f, 0.688},
       {0.066f, 0.079f, 0.102f, 0.146f, 0.200f, 0.244f, 0.282f, 0.309f, 0.308f, 0.278f, 0.231f, 0.178f,
        0.130f, 0.094f, 0.070f, 0.054f, 0.046f, 0.042f, 0.039f, 0.038f, 0.038f, 0.038f, 0.038f, 0.039f,
        0.039f, 0.040f, 0.041f, 0.042f, 0.044f, 0.045f, 0.046f, 0.046f, 0.048f, 0.052f, 0.057f, 0.065},
       {0.052f, 0.053f, 0.054f, 0.055f, 0.057f, 0.059f, 0.061f, 0.066f, 0.075f, 0.093f, 0.125f, 0.178f,
        0.246f, 0.307f, 0.337f, 0.334f, 0.317f, 0.293f, 0.262f, 0.230f, 0.198f, 0.165f, 0.135f, 0.115f,
        0.104f, 0.098f, 0.094f, 0.092f, 0.093f, 0.097f, 0.102f, 0.108f, 0.113f, 0.115f, 0.114f, 0.114},
       {0.050f, 0.049f, 0.048f, 0.047f, 0.047f, 0.047f, 0.047f, 0.047f, 0.046f, 0.045f, 0.044f, 0.044f,
        0.045f, 0.046f, 0.047f, 0.048f, 0.049f, 0.050f, 0.054f, 0.060f, 0.072f, 0.104f, 0.178f, 0.312f,
        0.467f, 0.581f, 0.644f, 0.675f, 0.690f, 0.698f, 0.706f, 0.715f, 0.724f, 0.730f, 0.734f, 0.738},
       {0.058f, 0.054f, 0.052f, 0.052f, 0.053f, 0.054f, 0.056f, 0.059f, 0.067f, 0.081f, 0.107f, 0.152f,
        0.225f, 0.336f, 0.462f, 0.559f, 0.616f, 0.650f, 0.672f, 0.694f, 0.710f, 0.723f, 0.731f, 0.739f,
        0.746f, 0.752f, 0.758f, 0.764f, 0.769f, 0.771f, 0.776f, 0.782f, 0.790f, 0.796f, 0.799f, 0.804},
       {0.145f, 0.195f, 0.283f, 0.346f, 0.362f, 0.354f, 0.334f, 0.306f, 0.276f, 0.248f, 0.218f, 0.190f,
        0.168f, 0.149f, 0.127f, 0.107f, 0.100f, 0.102f, 0.104f, 0.109f, 0.137f, 0.200f, 0.290f, 0.400f,
        0.516f, 0.615f, 0.687f, 0.732f, 0.760f, 0.774f, 0.783f, 0.793f, 0.803f, 0.812f, 0.817f, 0.825},
       {0.108f, 0.141f, 0.192f, 0.236f, 0.261f, 0.286f, 0.317f, 0.353f, 0.390f, 0.426f, 0.446f, 0.444f,
        0.423f, 0.385f, 0.337f, 0.283f, 0.231f, 0.185f, 0.146f, 0.118f, 0.101f, 0.090f, 0.082f, 0.076f,
        0.074f, 0.073f, 0.073f, 0.074f, 0.076f, 0.077f, 0.076f, 0.075f, 0.073f, 0.072f, 0.074f, 0.079},
       {0.189f, 0.255f, 0.423f, 0.660f, 0.811f, 0.862f, 0.877f, 0.884f, 0.891f, 0.896f, 0.899f, 0.904f,
        0.907f, 0.909f, 0.911f, 0.910f, 0.911f, 0.914f, 0.913f, 0.916f, 0.915f, 0.916f, 0.914f, 0.915f,
        0.918f, 0.919f, 0.921f, 0.923f, 0.924f, 0.922f, 0.922f, 0.925f, 0.927f, 0.930f, 0.930f, 0.933},
       {0.171f, 0.232f, 0.365f, 0.507f, 0.567f, 0.583f, 0.588f, 0.590f, 0.591f, 0.590f, 0.588f, 0.588f,
        0.589f, 0.589f, 0.591f, 0.590f, 0.590f, 0.590f, 0.589f, 0.591f, 0.590f, 0.590f, 0.587f, 0.585f,
        0.583f, 0.580f, 0.578f, 0.576f, 0.574f, 0.572f, 0.571f, 0.569f, 0.568f, 0.568f, 0.566f, 0.566},
       {0.144f, 0.192f, 0.272f, 0.331f, 0.350f, 0.357f, 0.361f, 0.363f, 0.363f, 0.361f, 0.359f, 0.358f,
        0.358f, 0.359f, 0.360f, 0.360f, 0.361f, 0.361f, 0.360f, 0.362f, 0.362f, 0.361f, 0.359f, 0.358f,
        0.355f, 0.352f, 0.350f, 0.348f, 0.345f, 0.343f, 0.340f, 0.338f, 0.335f, 0.334f, 0.332f, 0.331},
       {0.105f, 0.131f, 0.163f, 0.180f, 0.186f, 0.190f, 0.193f, 0.194f, 0.194f, 0.192f, 0.191f, 0.191f,
        0.191f, 0.192f, 0.192f, 0.192f, 0.192f, 0.192f, 0.192f, 0.193f, 0.192f, 0.192f, 0.191f, 0.189f,
        0.188f, 0.186f, 0.184f, 0.182f, 0.181f, 0.179f, 0.178f, 0.176f, 0.174f, 0.173f, 0.172f, 0.171},
       {0.068f, 0.077f, 0.084f, 0.087f, 0.089f, 0.090f, 0.092f, 0.092f, 0.091f, 0.090f, 0.090f, 0.090f,
        0.090f, 0.090f, 0.090f, 0.090f, 0.090f, 0.090f, 0.090f, 0.090f, 0.090f, 0.089f, 0.089f, 0.088f,
        0.087f, 0.086f, 0.086f, 0.085f, 0.084f, 0.084f, 0.083f, 0.083f, 0.082f, 0.081f, 0.081f, 0.081},
       {0.031f, 0.032f, 0.032f, 0.033f, 0.033f, 0.033f, 0.033f, 0.033f, 0.032f, 0.032f, 0.032f, 0.032f,
        0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f,
        0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.032f, 0.033}};

#endif   // MACBETH_DATA_H_
