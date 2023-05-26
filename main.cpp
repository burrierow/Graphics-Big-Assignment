/*
作者名称：王子浩202008010103，吕乐翔202008010115
计科2001
*/


#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Shader.h"
#include "Texture.h"


using namespace std;
struct keyboardcontroller
{
    bool UP = false;
    bool DOWN = false;
    bool LEFT = false;
    bool RIGHT = false;
};



keyboardcontroller keyboardCtl;


// 在 x 轴方向上，飞船移动的大小
float x_delta = 1.0f;
// 对应该方向上移动的次数
float x_press_num = 0.0f;
// 在 z 轴方向上，飞船移动的大小
float z_delta = 1.0f;
// 对应该方向上移动的次数
float z_press_num = 0.0f;


// 飞船的位置坐标
glm::vec3 craftPos = glm::vec3(0.0f, 0.0f, 0.0f);
// 行星和本地飞船旋转角度
float rotation = 0.0f;
// 飞船自身旋转角度
float rotation2 = 0.0f;
// 获取上一次的时间，用于计算旋转和其他事件的时间间隔
double prevTime = glfwGetTime();
// 在 x 轴方向上本地飞行器移动的大小
float x_delta2 = 0.5f;


struct UFO {
    glm::vec3 position;
    enum Status{intact=0, damaged=1, destroyed=2} status;
    float movement_magnitude = 0.5f;
    float x_movement_range[2];
};

// 屏幕设置800*600
GLFWwindow* window;
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;


std::vector<UFO> ufo_container;

//直接光源
float intensity = 1.0f;

bool space = false;

std::vector<glm::mat4> rocketPos;
std::vector<glm::mat4>::iterator it;

glm::vec3 PlanetPos = glm::vec3(0.0f, 0.0f, 100.0f);

Shader shader;

glm::vec3 cameraPos;//相机位置
const int n = 6;//东西数量 
GLuint vao[n];
GLuint vbo[n];
GLuint ebo[n];

Texture texture[7];

// 存储 obj 文件的结构体
struct Vertex {
    // 顶点位置坐标
    glm::vec3 position;
    // 顶点的 UV 坐标
    glm::vec2 uv;
    // 顶点法向量
    glm::vec3 normal;
};

// 模型结构体，包含一组顶点和索引列表
struct Model {
    // 顶点列表
    std::vector<Vertex> vertices;
    // 索引列表
    std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{


    // 包含了判断一个顶点是否已经出现的结构体
    struct V {
        // 顶点位置坐标、uv 坐标、法向量在各自 vector 中的下标位置
        unsigned int index_position, index_uv, index_normal;
        // 重载等于运算符，判断两个顶点是否相等
        bool operator == (const V& v) const {
            return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
        }
        // 重载小于运算符，用于排序
        bool operator < (const V& v) const {
            return (index_position < v.index_position) ||
                (index_position == v.index_position && index_uv < v.index_uv) ||
                (index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
        }
    };
    // 将 obj 文件每行读入后暂存顶点位置、顶点 UV 坐标、顶点法向量的 vector 中
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    // 用哈希表存储顶点，可以避免重复顶点，并快速查找已经存在的顶点在顶点列表中的索引
    std::map<V, unsigned int> temp_vertices;


    Model model;
    unsigned int num_vertices = 0;

    //std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

    std::ifstream file;
    file.open(objPath);

    // Check for Error
    if (file.fail()) {
        std::cerr << "No! Fail to open the file! " << std::endl;
        exit(1);
    }
    // Load();
    while (!file.eof()) {
        
        char lineHeader[128];
        file >> lineHeader;

        // 读取 obj 文件中的顶点数据
        if (strcmp(lineHeader, "v") == 0) {
            // 读取顶点位置坐标
            glm::vec3 position;
            file >> position.x >> position.y >> position.z;
            // 对特定模型进行位置偏移操作
            if (objPath == "resources/object/spacecraft.ob") {
                position.y = position.y - 220.0f;
                position.z = position.z - 120.0f;
            }
            temp_positions.push_back(position);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            // 读取顶点的 UV 坐标
            glm::vec2 uv;
            file >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            // 读取顶点法向量
            glm::vec3 normal;
            file >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            // 面元素
            V vertices[3];
            // 读取 obj 文件中的面（三角形）数据
            for (int i = 0; i < 3; i++) {
                char ch;
                file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
            }

            for (int i = 0; i < 3; i++) {
                if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
                    // 顶点还没有出现过
                    Vertex vertex;
                    vertex.position = temp_positions[vertices[i].index_position - 1];
                    vertex.uv = temp_uvs[vertices[i].index_uv - 1];
                    vertex.normal = temp_normals[vertices[i].index_normal - 1];

                    model.vertices.push_back(vertex);
                    model.indices.push_back(num_vertices);
                    temp_vertices[vertices[i]] = num_vertices;
                    num_vertices += 1;
                }
                else {
                    // 重复的顶点，直接复用已经存在的顶点，避免重复
                    unsigned int index = temp_vertices[vertices[i]];
                    model.indices.push_back(index);
                }
            } // for
        } // else if
        else {
            // 啥也不是
            char stupidBuffer[1024];
            file.getline(stupidBuffer, 1024);
        }
    }
    file.close();

    std::cout << "在obj文件内有 " << num_vertices << " 个顶点\n" << std::endl;
    
    return model;
}

//加载模型
Model spacecraft = loadOBJ("resources/object/spacecraft.obj");
Model rock = loadOBJ("resources/object/rock.obj");
Model planet = loadOBJ("resources/object/planet.obj");
Model craft = loadOBJ("resources/object/Craft.obj");
Model skybox = loadOBJ("resources/skybox/skybox.obj");
Model rocket = loadOBJ("resources/object/ring.obj");



void sendDataToOpenGL()
{
    // 加载 obj 文件之后，将顶点和索引数据绑定到 VAO 和 VBO 上
    
    // 绑定太空飞船模型
    glGenVertexArrays(1, &vao[0]);
    glBindVertexArray(vao[0]);


    /*
    这段代码用于创建和绑定VBO（顶点缓冲对象）以存储太空飞船的顶点数据。
    使用 glGenBuffers 函数生成一个新的 VBO 标识符，并将其存储在 vbo 数组的第一个元素中
    在该段代码中，vbo 数组应该是通过 glGenBuffers 函数提前定义好的一个数组，里面存储了可以存储多个 VBO 的标识符。
    使用 glBindBuffer 函数将 VBO 绑定到 GL_ARRAY_BUFFER 目标上下文（即告诉 OpenGL，下一步的 VBO 操作将针对 GL_ARRAY_BUFFER）。
    使用 glBufferData 函数将太空飞船的顶点数据拷贝到 VBO 中。
    其中，glBufferData 函数的第一个参数代表 GL_ARRAY_BUFFER 的目标上下文，
    第二个参数是需要拷贝进 VBO 的数据大小（即顶点缓冲区的大小），第三个参数是需要拷贝的数据指针，第四个参数是指定该 VBO 的数据使用模式。
    在这里，我们使用了 GL_STATIC_DRAW 数据使用模式，表示我们不希望在运行时修改缓冲区内的数据。
    */
    glGenBuffers(1, &vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, spacecraft.vertices.size() * sizeof(Vertex), &spacecraft.vertices[0], GL_STATIC_DRAW);


    // 设置 vertex shader 中的顶点属性
    // 位置坐标
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), 
        (void*)offsetof(Vertex, position) // 数组缓冲偏移
    );

    //vertex uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 
        2, 
        GL_FLOAT, 
        GL_FALSE, 
        sizeof(Vertex), 
        (void*)offsetof(Vertex, uv) 
    );

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 
        3, 
        GL_FLOAT, 
        GL_FALSE, 
        sizeof(Vertex),
        (void*)offsetof(Vertex, normal) 
    );

    glGenBuffers(1, &ebo[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, spacecraft.indices.size() * sizeof(unsigned int), &spacecraft.indices[0], GL_STATIC_DRAW);
    
/*
    在OpenGL程序中，将顶点数据和着色器程序联系起来需要使用缓冲区对象（Buffer Object）和顶点数组对象（Vertex Array Object）。在加载OBJ文件之后，需要将OBJ文件中的顶点和索引数据绑定到缓冲区对象上，并将缓冲区对象与顶点数组对象联系起来，方便后续的渲染。
    在绑定顶点数据时，需要指定顶点数组对象中每个属性的位置，这些位置将作为在着色器程序中声明的顶点属性的索引，以便OpenGL知道如何将属性数据传递给着色器程序。在上面的代码中，通过glVertexAttribPointer函数来指定位置、大小、类型、是否进行规范化，属性的偏移量等参数。
    而与element buffer object（EBO）有关的代码用于存储和绑定此模型的索引数据，EBO是用来存储几何图形的顶点索引的缓冲区对象。通过glBufferData函数将数据上传到缓冲区对象中。这里通过glGenBuffers函数生成一个新的缓冲区标识并将其绑定到当前上下文中，最后调用glBufferData函数将数据传输到缓冲区对象。
    */


    //绑定 rock
    glGenVertexArrays(1, &vao[1]);
    glBindVertexArray(vao[1]);

    glGenBuffers(1, &vbo[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, rock.vertices.size() * sizeof(Vertex), &rock.vertices[0], GL_STATIC_DRAW);

    // 设置顶点属性数组和EBO

 // 位置坐标
    glEnableVertexAttribArray(0); // 先启用顶点属性数组
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride 步长，即每一个顶点所占用的字节数
        (void*)offsetof(Vertex, position) // array buffer offset，每个顶点数据中，位置数据在开始位置的偏移量
    );

    // 纹理坐标
    glEnableVertexAttribArray(1); // 启用纹理坐标
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride 步长，即每一个顶点所占用的字节数
        (void*)offsetof(Vertex, uv) // array buffer offset，每个顶点数据中，纹理坐标在开始位置的偏移量
    );

    // 法向量
    glEnableVertexAttribArray(2); // 启用法向量
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride 步长，即每一个顶点所占用的字节数
        (void*)offsetof(Vertex, normal) // array buffer offset，每个顶点数据中，法向量在开始位置的偏移量
    );

    // 绑定圆形石头的EBO
    glGenBuffers(1, &ebo[1]); // 生成新的EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]); // 绑定到当前上下文
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock.indices.size() * sizeof(unsigned int), &rock.indices[0], GL_STATIC_DRAW); // 将数据传输到EBO缓冲区对象中

    /*
    在OpenGL中，每个顶点可以包含多个属性，例如位置、法向量、颜色和纹理坐标等。使用VBO（Vertex Buffer Object）存储顶点数据之后，需要使用VAO（Vertex Array Object）来指定如何从VBO中获取顶点数据，并将数据传递到着色器程序中。
上述代码片段中，首先启用顶点属性数组，然后使用glVertexAttribPointer函数指定顶点属性，在这里分别指定了位置、纹理坐标和法向量。它们的位置值分别为0、1和2，大小分别为3、2和3（分别代表每个顶点一共有几个变量），类型为GL_FLOAT。最后两个参数分别是步长和偏移量，步长指的是连续顶点间的字节数，偏移量指的是该顶点属性在缓冲区中的偏移量，这个偏移量用使用offsetof函数来获取
在此代码片段中，还使用glGenBuffers生成一个新的EBO（Element Buffer Object）并将其绑定到当前上下文中。EBO是用来存储几何图形的索引的缓冲区对象，有效地减少了重复数据的存储和渲染次数。我们可以将索引数据与VAO关联，以便后续的渲染时快速绘制几何图形。最后使用glBufferData函数将数据上传到EBO缓冲区对象中。
    */


    //bind planet
    glGenVertexArrays(1, &vao[2]);
    glBindVertexArray(vao[2]);

    glGenBuffers(1, &vbo[2]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, planet.vertices.size() * sizeof(Vertex), &planet.vertices[0], GL_STATIC_DRAW);

    //vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );

    //vertex uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planet.indices.size() * sizeof(unsigned int), &planet.indices[0], GL_STATIC_DRAW);
    
    //bind local vehicle
    glGenVertexArrays(1, &vao[3]);
    glBindVertexArray(vao[3]);

    glGenBuffers(1, &vbo[3]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, craft.vertices.size() * sizeof(Vertex), &craft.vertices[0], GL_STATIC_DRAW);

    //vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );

    //vertex uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[3]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, craft.indices.size() * sizeof(unsigned int), &craft.indices[0], GL_STATIC_DRAW);

    //bind skybox
    glGenVertexArrays(1, &vao[4]);
    glBindVertexArray(vao[4]);

    glGenBuffers(1, &vbo[4]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, skybox.vertices.size() * sizeof(Vertex), &skybox.vertices[0], GL_STATIC_DRAW);

    //vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );

    //vertex uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[4]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skybox.indices.size() * sizeof(unsigned int), &skybox.indices[0], GL_STATIC_DRAW);


    //bind skybox
    glGenVertexArrays(1, &vao[4]);
    glBindVertexArray(vao[4]);

    glGenBuffers(1, &vbo[4]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, skybox.vertices.size() * sizeof(Vertex), &skybox.vertices[0], GL_STATIC_DRAW);

    //vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );

    //vertex uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[4]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skybox.indices.size() * sizeof(unsigned int), &skybox.indices[0], GL_STATIC_DRAW);

    //bind rocket
    glGenVertexArrays(1, &vao[5]);
    glBindVertexArray(vao[5]);

    glGenBuffers(1, &vbo[5]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glBufferData(GL_ARRAY_BUFFER, rocket.vertices.size() * sizeof(Vertex), &rocket.vertices[0], GL_STATIC_DRAW);

    //vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );

    //vertex uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[5]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[5]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rocket.indices.size() * sizeof(unsigned int), &rocket.indices[0], GL_STATIC_DRAW);
    
    //Load textures
    texture[0].setupTexture("resources/texture/spacecraftTexture.bmp");
    texture[1].setupTexture("resources/texture/rockTexture.bmp");
    texture[2].setupTexture("resources/texture/earthTexture.bmp");
    texture[3].setupTexture("resources/texture/vehicleTexture.bmp");
    texture[4].setupTexture("resources/skybox/all_face_textures.png");
    texture[5].setupTexture("resources/texture/earthNormal.bmp");
    texture[6].setupTexture("resources/rocket/rocket.png");
    texture[7].setupTexture("resources/texture/vehicleTextureRed.png");
    
}

// 环形中石头的数量
const int num = 200;
// 石头大小上限
float scaleMax = 0.7f;
// 石头大小下限
float scaleMin = 0.1f;
// 存储不同石头大小的数组
float scale[num];
// 石头Y坐标上限
float yMax = 8.0f;
// 石头Y坐标下限
float yMin = 5.0f;
// 存储不同石头Y坐标的数组
float y[num];
// 环形大小上限
float circleMax = 135.0f;
// 环形大小下限
float circleMin = 65.0f;
// 存储不同石头Z坐标的数组
float z[num];
// 存储不同石头X坐标的数组
float x[num];


void initializedGL(void) //run only once
{
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW 不太 OK." << std::endl;
    }

    std::cout << "作者 : 王子浩、吕乐翔 " << std::endl;
    std::cout << "学号 : 王（202008010103）、吕（202008010115）" << std::endl;

    sendDataToOpenGL();

    // 设置ufo
    UFO aUFO;
    aUFO.position = glm::vec3(0.0f, 0.0f, 20.0f);
    aUFO.status = UFO::intact;
    aUFO.x_movement_range[0] = -15.0f;
    aUFO.x_movement_range[1] = 15.0f;
    aUFO.movement_magnitude = 0.3f;
    ufo_container.push_back(aUFO);
    
    //设置相机位置
    cameraPos = glm::vec3(0.0f, 2.5f, -6.5f);
    //设置顶点着色器和片段着色器
    shader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");

    /*
     启用OpenGL的深度测试和剔除面功能。
深度测试是OpenGL的一种基本的渲染技术，用来处理场景中物体的遮挡关系。在深度测试中，OpenGL会根据物体的深度值（一般是从相机到物体之间的距离）判断物体是否可见。启用深度测试可以通过 glEnable(GL_DEPTH_TEST) 来实现。
剔除面是OpenGL的另一种渲染技术，用来忽略对于场景的某些面的渲染，这样可以提高渲染速度。当物体在场景中不可见的背面，没有必要在渲染过程中绘制它们，因为它们永远不会被看到。在OpenGL中启用剔除面可以通过 glEnable(GL_CULL_FACE) 来实现。
    */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);



    //随机位置放石头
    for (int i = 0; i < num; i++) {
        scale[i] = (scaleMax - scaleMin) * rand() / (RAND_MAX + 1.0) + scaleMin;
        y[i] = (yMax - yMin) * rand() / (RAND_MAX + 1.0) + yMin;
        z[i] = (circleMax - circleMin) * rand() / (RAND_MAX + 1.0) + circleMin;
        x[i] = sqrt(1225 - (z[i] - 100) * (z[i] - 100));
        if (i % 2 == 0)
            x[i] *= -1;
    };
}

void paintGL(void)  
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f); //specify the background color, this is just an example
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    shader.use();
    //设置光源的位置和颜色等光照信息
    //光源颜色
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
    glm::vec3 ambientColor = lightColor * glm::vec3(0.05f);

    //定向光源信息
    shader.setVec3("dirLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
    shader.setVec3("dirLight.ambient", ambientColor);
    shader.setVec3("dirLight.diffuse", diffuseColor);
    shader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setFloat("dirLight.intensity", intensity);

    //点光源信息
    shader.setVec3("pointLight.position", glm::vec3(15.0f, 7.0f, 35.0f));
    shader.setVec3("pointLight.ambient", ambientColor);
    shader.setVec3("pointLight.diffuse", diffuseColor);
    shader.setVec3("pointLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setFloat("pointLight.constant", 0.1f);
    shader.setFloat("pointLight.linear", 0.0014f);
    shader.setFloat("pointLight.quadratic", 0.00032f);

    //设置变换矩阵
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    //设置透视投影
    projectionMatrix = glm::perspective(glm::radians(75.0f), 1.0f, 0.1f, 500.0f);
    shader.setMat4("projectionMatrix", projectionMatrix);

    //0.05秒事件扳机
    double crntTime = glfwGetTime();
    if (crntTime - prevTime >= 0.05) {
        //自旋转
        rotation += 1.0f;
        prevTime = crntTime;
        
        /*
        其中，ufo_container 是包含了所有 UFO 对象的容器。对于每个 UFO，它的位置 x 坐标会根据 movement_magnitude 增加或减少，
        然后当它越过了这个运动范围的边界时，movement_magnitude 就会取相反数，从而让 UFO 反向运动。这样一来，
        所有 UFO 对象就会在它们各自的运动范围内自由运动，从而增加了场景的真实感。
        */
        for (std::vector<UFO>::iterator iter = ufo_container.begin(); iter != ufo_container.end(); iter++) {
            if (iter->position.x >= iter->x_movement_range[0] && iter->position.x <= iter->x_movement_range[1]) {
                
                iter->position.x += iter->movement_magnitude;//move forward on the track
            } else {
                iter->movement_magnitude *= -1.0f;
                iter->position.x += iter->movement_magnitude;//move backward on the track
            }
        }
        
        
        //上下左右
        if (keyboardCtl.LEFT)
            x_press_num += 1.0f;
        if (keyboardCtl.RIGHT)
            x_press_num -= 1.0f;
        if (keyboardCtl.UP)
            z_press_num += 1.0f;
        if (keyboardCtl.DOWN)
            z_press_num -= 1.0f;
        double xpos, ypos;//鼠标位置
        glfwGetCursorPos(window, &xpos, &ypos);
        rotation2 = (xpos-400) / 800 * 360;//rotation angle for spacecraft
    }
    
    for (it = rocketPos.begin();it != rocketPos.end();it++) {
        glm::mat4 modelMatrixR = *it;
        modelMatrixR = glm::translate(modelMatrixR, glm::vec3(0.0f, 0.0f, 0.5f * 800.0f));
        std::replace(rocketPos.begin(), rocketPos.end(), *it, modelMatrixR);
    }
    

    //设置与宇宙飞船相关的静止摄像机
    viewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    viewMatrix = glm::rotate(viewMatrix, glm::radians(rotation2), glm::vec3(0.0f, 0.5f, 0.0f));
    viewMatrix = glm::translate(viewMatrix, glm::vec3(-x_delta * x_press_num , 0.0f, -z_delta * z_press_num));
    //发送给着色器
    shader.setVec3("eyePositionWorld", cameraPos);
    shader.setMat4("viewMatrix", viewMatrix);
    shader.setInt("normalMapping_flag", 0);//默认禁用法线贴图
    shader.setInt("light_flag", 1);

    

    //平移和旋转
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_delta * x_press_num, 0.0f, z_delta * z_press_num));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
    

    /*
    这段代码是用来将一个模型缩放到较小的尺寸，以便更好地在相机视角下显示。
    具体来说，当 space 为真时，在模型变换矩阵 modelMatrix 上应用一个缩放变换，缩小模型的大小，
    并将该变换后的矩阵压入 rocketPos 向量。然后将 space 设为假，以避免进入下一个循环时再次执行这段代码。
    最后，将原始矩阵中的缩放变换还原，以避免将这个缩放应用到接下来的渲染中。
    */
    if (space) {
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f / 1200.0f));
        rocketPos.push_back(modelMatrix);
        space = false;
        modelMatrix = glm::scale(modelMatrix, glm::vec3(800.0f));
    }
    
    /*
    将模型矩阵 modelMatrix 按照缩放因子 1.0f / 400.0f 进行缩放操作，使得模型变为原来的 1/400 大小，使用 glm::scale 函数进行操作
    将缩放后的矩阵 modelMatrix 传递给着色器 shader 以更新模型变换矩阵，使用 shader.setMat4 函数进行操作
    将纹理 texture[0] 绑定到纹理单元 0 上，使用 texture[0].bind 函数进行操作
    设置纹理采样器的值 myTextureSampler0 为 0，使用 shader.setInt 函数进行操作
    绑定顶点数组 vao[0]，使用 glBindVertexArray 函数进行操作
    调用 glDrawElements 函数进行绘制操作。其中使用 GL_TRIANGLES 绘制模式，并传递模型索引数据 spacecraft.indices 和偏移量参数 0
    最后，将模型矩阵 modelMatrix 按照顺序进行旋转和平移操作：首先，根据旋转角度 rotation2 绕 y 轴旋转一定角度，
    使用 glm::rotate 函数进行操作；然后，根据变换量 (-x_delta * x_press_num, 0.0f, -z_delta * z_press_num) 进行平移操作，使用 glm::translate 函数进行操作
    */
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f / 400.0f));
    shader.setMat4("modelMatrix", modelMatrix);
    texture[0].bind(0);
    shader.setInt("myTextureSampler0", 0);
    glBindVertexArray(vao[0]);
    glDrawElements(GL_TRIANGLES, (int)spacecraft.indices.size(), GL_UNSIGNED_INT, 0);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(200.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-x_delta * x_press_num, 0.0f, -z_delta * z_press_num));
    
    /*
    通过循环遍历多个火箭模型的位置信息，使用迭代器 it 进行操作
    获取当前火箭模型的位置向量 rocketLocVec，即火箭模型变换矩阵的第四列向量，使用 glm::vec3 函数进行操作
    将火箭模型的纹理 texture[6] 绑定到纹理单元 0 上，使用 texture[6].bind 函数进行操作
    设置纹理采样器的值 myTextureSampler0 为 0，使用 shader.setInt 函数进行操作
    绑定火箭模型的顶点数组 vao[5]，使用 glBindVertexArray 函数进行操作
    根据当前火箭模型的变换矩阵 *it 更新模型变换矩阵的值，使用 shader.setMat4 函数进行操作
    使用 glDrawElements 函数进行绘制操作。其中使用 GL_TRIANGLES 绘制模式，并传递火箭模型的索引数据 rocket.indices 和偏移量参数 0，完成火箭模型的绘制操作
    这样，通过循环遍历多个火箭模型的位置信息，并绑定相应的纹理和顶点数组，可以对多个火箭模型进行绘制，完成火箭游戏中一帧的渲染。
    */
    for (it = rocketPos.begin();it != rocketPos.end();it++) {
        // Remove if the rocket is out of skybox
        glm::vec3 rocketLocVec = glm::vec3((*it)[3]);
        
        texture[6].bind(0);
        shader.setInt("myTextureSampler0", 0);
        glBindVertexArray(vao[5]);
        shader.setMat4("modelMatrix", *it);
        glDrawElements(GL_TRIANGLES, (int)rocket.indices.size(), GL_UNSIGNED_INT, 0);
    }
    
    //设置石头
    texture[1].bind(0);
    shader.setInt("myTextureSampler0", 0);
    glBindVertexArray(vao[1]);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 6.5f, 100.0f));
    for (int i = 0; i < num; i++) {
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x[i], y[i], z[i]));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -6.5f, -100.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale[i]));
        shader.setMat4("modelMatrix", modelMatrix);
        glDrawElements(GL_TRIANGLES, (int)rock.indices.size(), GL_UNSIGNED_INT, 0);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1/scale[i]));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 6.5f, 100.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-x[i], -y[i], -z[i]));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -6.5f, -100.0f));
    
    /*
         Planet
     */
    shader.setInt("normalMapping_flag", 1);

    modelMatrix = glm::translate(modelMatrix, PlanetPos);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f));
    shader.setMat4("modelMatrix", modelMatrix);
    texture[5].bind(0);
    shader.setInt("myTextureSampler1", 0);
    texture[2].bind(0);
    shader.setInt("myTextureSampler0", 0);
    glBindVertexArray(vao[2]);
    glDrawElements(GL_TRIANGLES, (int)planet.indices.size(), GL_UNSIGNED_INT, 0);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -100.0f));

    /*
        UFO
     */
    shader.setInt("normalMapping_flag", 0);

    for (std::vector<UFO>::iterator iter = ufo_container.begin();
         iter != ufo_container.end(); iter++) {
        
        if (iter->status == UFO::intact) {
            texture[3].bind(0);
        } else if (iter->status == UFO::damaged) {
            texture[7].bind(0);
        } else if (iter->status == UFO::destroyed) {
            continue;
        }
        
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        modelMatrix = glm::translate(modelMatrix, iter->position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("modelMatrix", modelMatrix);
        shader.setInt("myTextureSampler0", 0);
        glBindVertexArray(vao[3]);
        glDrawElements(GL_TRIANGLES, (int)craft.indices.size(), GL_UNSIGNED_INT, 0);
        
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::translate(modelMatrix,  iter->position * -1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f));
       
        

    }

   

    //disable light
    shader.setInt("light_flag", 0);
    //set model matrix and draw skybox
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(4.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -30.0f, 0.0f));
    shader.setMat4("modelMatrix", modelMatrix);
    texture[4].bind(0);
    shader.setInt("myTextureSampler0", 0);
    glBindVertexArray(vao[4]);
    glDrawElements(GL_TRIANGLES, (int)skybox.indices.size(), GL_UNSIGNED_INT, 0);
    
   
}


/*
framebuffer_size_callback 用于监听窗口大小的变化，当窗口大小发生变化时，OpenGL 视口也需要重新调整。glViewport 函数用于重新设置视口大小，输入参数为左下角的位置和宽度高度。
mouse_button_callback 用于监听鼠标按钮的状态，当鼠标按钮状态发生变化时，可以根据需要进行相应处理。
cursor_position_callback 用于监听鼠标光标位置的变化，当光标位置发生变化时，可以根据需要进行相应处理。
scroll_callback 用于监听鼠标滚轮的滚动，当滚轮滚动时，可以根据需要进行相应处理。
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    cout << "鼠标按了" << endl;
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    cout << "光标变了" << endl;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cout << "滚轮动了" << endl;
}



/*
当按下 ESCAPE 键时，调用 glfwSetWindowShouldClose 函数关闭窗口
当按下 LEFT 键时，将 keyboardCtl 结构体中的 LEFT 标志设为真，表示左方向键正在被按下
当释放 LEFT 键时，将相应的 keyboardCtl 结构体中的标志设为假，表示左方向键已经被释放
当按下 RIGHT，UP，DOWN 键时，也同样进行类似的处理
当按下 W 键时，如果光源强度仍然小于 1.0，增加其强度 0.1
当按下 S 键时，如果光源强度仍然大于 0.0，减小其强度 0.1
当按下 SPACE 键时，将 space 标志设为 true，表示此时需要执行一次模型缩放操作
当按下 R 键时，遍历所有的 ufo_container 中的 UFO 对象，将它们的状态设为初始状态 intact，并将它们的位置重置为 (0.0f, 0.0f, 20.0f)
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        keyboardCtl.LEFT = true;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
        keyboardCtl.LEFT = false;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        keyboardCtl.RIGHT = true;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
        keyboardCtl.RIGHT = false;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        keyboardCtl.UP = true;
    }
    if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
        keyboardCtl.UP = false;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        keyboardCtl.DOWN = true;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
        keyboardCtl.DOWN = false;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        if (intensity < 1.0f)
            intensity += 0.1f;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        if (intensity > 0.0f)
            intensity -= 0.1f;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (!space) {
            space = true;
        }
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        for (std::vector<UFO>::iterator iter = ufo_container.begin();
             iter != ufo_container.end(); iter++) {
            iter->status = UFO::intact;
            iter->position = glm::vec3(0.0f, 0.0f, 20.0f);
        }
    }
}

int main(int argc, char* argv[])
{

    
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // 初始化 GLFW 库

    //规定版本号，规定兼容模式向后兼容。
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // 配置 GLFW 库，这里设置 OpenGL 版本为 3.3 并使用核心(profiler)模式

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "wzh&llx's final Graphics work", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;   // 若窗口创建失败，则输出错误信息
        glfwTerminate();
        return -1;
    }
    // 创建窗口

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    // 使窗口的上下文为当前上下文

    /*register callback functions*/
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // 注册缓冲大小回调函数
    glfwSetKeyCallback(window, key_callback);  // 注册键盘回调函数
    glfwSetScrollCallback(window, scroll_callback);  // 注册滚动回调函数
    glfwSetCursorPosCallback(window, cursor_position_callback);  // 注册光标位置回调函数
    glfwSetMouseButtonCallback(window, mouse_button_callback);  // 注册鼠标按键回调函数
    // 注册回调函数

    initializedGL();   // 自定义初始化函数


    /*
    while 循环，它通过调用 paintGL() 渲染场景，并在每次渲染过程结束后使用 glfwSwapBuffers() 函数交换前后缓冲区，从而显示出渲染结果。
    此外，这段代码也注册了一些回调函数。
    */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */   // 渲染区域
        paintGL();  // 自定义渲染函数

        /* Swap front and back buffers */
        glfwSwapBuffers(window);   // 交换前后缓冲区

        /* Poll for and process events */
        glfwPollEvents();   // 处理事件
    }

    glfwTerminate();  // 释放 GLFW 资源

    return 0;
}