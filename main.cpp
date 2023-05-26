/*
�������ƣ����Ӻ�202008010103��������202008010115
�ƿ�2001
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


// �� x �᷽���ϣ��ɴ��ƶ��Ĵ�С
float x_delta = 1.0f;
// ��Ӧ�÷������ƶ��Ĵ���
float x_press_num = 0.0f;
// �� z �᷽���ϣ��ɴ��ƶ��Ĵ�С
float z_delta = 1.0f;
// ��Ӧ�÷������ƶ��Ĵ���
float z_press_num = 0.0f;


// �ɴ���λ������
glm::vec3 craftPos = glm::vec3(0.0f, 0.0f, 0.0f);
// ���Ǻͱ��طɴ���ת�Ƕ�
float rotation = 0.0f;
// �ɴ�������ת�Ƕ�
float rotation2 = 0.0f;
// ��ȡ��һ�ε�ʱ�䣬���ڼ�����ת�������¼���ʱ����
double prevTime = glfwGetTime();
// �� x �᷽���ϱ��ط������ƶ��Ĵ�С
float x_delta2 = 0.5f;


struct UFO {
    glm::vec3 position;
    enum Status{intact=0, damaged=1, destroyed=2} status;
    float movement_magnitude = 0.5f;
    float x_movement_range[2];
};

// ��Ļ����800*600
GLFWwindow* window;
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;


std::vector<UFO> ufo_container;

//ֱ�ӹ�Դ
float intensity = 1.0f;

bool space = false;

std::vector<glm::mat4> rocketPos;
std::vector<glm::mat4>::iterator it;

glm::vec3 PlanetPos = glm::vec3(0.0f, 0.0f, 100.0f);

Shader shader;

glm::vec3 cameraPos;//���λ��
const int n = 6;//�������� 
GLuint vao[n];
GLuint vbo[n];
GLuint ebo[n];

Texture texture[7];

// �洢 obj �ļ��Ľṹ��
struct Vertex {
    // ����λ������
    glm::vec3 position;
    // ����� UV ����
    glm::vec2 uv;
    // ���㷨����
    glm::vec3 normal;
};

// ģ�ͽṹ�壬����һ�鶥��������б�
struct Model {
    // �����б�
    std::vector<Vertex> vertices;
    // �����б�
    std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{


    // �������ж�һ�������Ƿ��Ѿ����ֵĽṹ��
    struct V {
        // ����λ�����ꡢuv ���ꡢ�������ڸ��� vector �е��±�λ��
        unsigned int index_position, index_uv, index_normal;
        // ���ص�����������ж����������Ƿ����
        bool operator == (const V& v) const {
            return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
        }
        // ����С�����������������
        bool operator < (const V& v) const {
            return (index_position < v.index_position) ||
                (index_position == v.index_position && index_uv < v.index_uv) ||
                (index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
        }
    };
    // �� obj �ļ�ÿ�ж�����ݴ涥��λ�á����� UV ���ꡢ���㷨������ vector ��
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    // �ù�ϣ��洢���㣬���Ա����ظ����㣬�����ٲ����Ѿ����ڵĶ����ڶ����б��е�����
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

        // ��ȡ obj �ļ��еĶ�������
        if (strcmp(lineHeader, "v") == 0) {
            // ��ȡ����λ������
            glm::vec3 position;
            file >> position.x >> position.y >> position.z;
            // ���ض�ģ�ͽ���λ��ƫ�Ʋ���
            if (objPath == "resources/object/spacecraft.ob") {
                position.y = position.y - 220.0f;
                position.z = position.z - 120.0f;
            }
            temp_positions.push_back(position);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            // ��ȡ����� UV ����
            glm::vec2 uv;
            file >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            // ��ȡ���㷨����
            glm::vec3 normal;
            file >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            // ��Ԫ��
            V vertices[3];
            // ��ȡ obj �ļ��е��棨�����Σ�����
            for (int i = 0; i < 3; i++) {
                char ch;
                file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
            }

            for (int i = 0; i < 3; i++) {
                if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
                    // ���㻹û�г��ֹ�
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
                    // �ظ��Ķ��㣬ֱ�Ӹ����Ѿ����ڵĶ��㣬�����ظ�
                    unsigned int index = temp_vertices[vertices[i]];
                    model.indices.push_back(index);
                }
            } // for
        } // else if
        else {
            // ɶҲ����
            char stupidBuffer[1024];
            file.getline(stupidBuffer, 1024);
        }
    }
    file.close();

    std::cout << "��obj�ļ����� " << num_vertices << " ������\n" << std::endl;
    
    return model;
}

//����ģ��
Model spacecraft = loadOBJ("resources/object/spacecraft.obj");
Model rock = loadOBJ("resources/object/rock.obj");
Model planet = loadOBJ("resources/object/planet.obj");
Model craft = loadOBJ("resources/object/Craft.obj");
Model skybox = loadOBJ("resources/skybox/skybox.obj");
Model rocket = loadOBJ("resources/object/ring.obj");



void sendDataToOpenGL()
{
    // ���� obj �ļ�֮�󣬽�������������ݰ󶨵� VAO �� VBO ��
    
    // ��̫�շɴ�ģ��
    glGenVertexArrays(1, &vao[0]);
    glBindVertexArray(vao[0]);


    /*
    ��δ������ڴ����Ͱ�VBO�����㻺������Դ洢̫�շɴ��Ķ������ݡ�
    ʹ�� glGenBuffers ��������һ���µ� VBO ��ʶ����������洢�� vbo ����ĵ�һ��Ԫ����
    �ڸöδ����У�vbo ����Ӧ����ͨ�� glGenBuffers ������ǰ����õ�һ�����飬����洢�˿��Դ洢��� VBO �ı�ʶ����
    ʹ�� glBindBuffer ������ VBO �󶨵� GL_ARRAY_BUFFER Ŀ�������ģ������� OpenGL����һ���� VBO ��������� GL_ARRAY_BUFFER����
    ʹ�� glBufferData ������̫�շɴ��Ķ������ݿ����� VBO �С�
    ���У�glBufferData �����ĵ�һ���������� GL_ARRAY_BUFFER ��Ŀ�������ģ�
    �ڶ�����������Ҫ������ VBO �����ݴ�С�������㻺�����Ĵ�С������������������Ҫ����������ָ�룬���ĸ�������ָ���� VBO ������ʹ��ģʽ��
    ���������ʹ���� GL_STATIC_DRAW ����ʹ��ģʽ����ʾ���ǲ�ϣ��������ʱ�޸Ļ������ڵ����ݡ�
    */
    glGenBuffers(1, &vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, spacecraft.vertices.size() * sizeof(Vertex), &spacecraft.vertices[0], GL_STATIC_DRAW);


    // ���� vertex shader �еĶ�������
    // λ������
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), 
        (void*)offsetof(Vertex, position) // ���黺��ƫ��
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
    ��OpenGL�����У����������ݺ���ɫ��������ϵ������Ҫʹ�û���������Buffer Object���Ͷ����������Vertex Array Object�����ڼ���OBJ�ļ�֮����Ҫ��OBJ�ļ��еĶ�����������ݰ󶨵������������ϣ����������������붥�����������ϵ�����������������Ⱦ��
    �ڰ󶨶�������ʱ����Ҫָ���������������ÿ�����Ե�λ�ã���Щλ�ý���Ϊ����ɫ�������������Ķ������Ե��������Ա�OpenGL֪����ν��������ݴ��ݸ���ɫ������������Ĵ����У�ͨ��glVertexAttribPointer������ָ��λ�á���С�����͡��Ƿ���й淶�������Ե�ƫ�����Ȳ�����
    ����element buffer object��EBO���йصĴ������ڴ洢�Ͱ󶨴�ģ�͵��������ݣ�EBO�������洢����ͼ�εĶ��������Ļ���������ͨ��glBufferData�����������ϴ��������������С�����ͨ��glGenBuffers��������һ���µĻ�������ʶ������󶨵���ǰ�������У�������glBufferData���������ݴ��䵽����������
    */


    //�� rock
    glGenVertexArrays(1, &vao[1]);
    glBindVertexArray(vao[1]);

    glGenBuffers(1, &vbo[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, rock.vertices.size() * sizeof(Vertex), &rock.vertices[0], GL_STATIC_DRAW);

    // ���ö������������EBO

 // λ������
    glEnableVertexAttribArray(0); // �����ö�����������
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride ��������ÿһ��������ռ�õ��ֽ���
        (void*)offsetof(Vertex, position) // array buffer offset��ÿ�����������У�λ�������ڿ�ʼλ�õ�ƫ����
    );

    // ��������
    glEnableVertexAttribArray(1); // ������������
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride ��������ÿһ��������ռ�õ��ֽ���
        (void*)offsetof(Vertex, uv) // array buffer offset��ÿ�����������У����������ڿ�ʼλ�õ�ƫ����
    );

    // ������
    glEnableVertexAttribArray(2); // ���÷�����
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride ��������ÿһ��������ռ�õ��ֽ���
        (void*)offsetof(Vertex, normal) // array buffer offset��ÿ�����������У��������ڿ�ʼλ�õ�ƫ����
    );

    // ��Բ��ʯͷ��EBO
    glGenBuffers(1, &ebo[1]); // �����µ�EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]); // �󶨵���ǰ������
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock.indices.size() * sizeof(unsigned int), &rock.indices[0], GL_STATIC_DRAW); // �����ݴ��䵽EBO������������

    /*
    ��OpenGL�У�ÿ��������԰���������ԣ�����λ�á�����������ɫ����������ȡ�ʹ��VBO��Vertex Buffer Object���洢��������֮����Ҫʹ��VAO��Vertex Array Object����ָ����δ�VBO�л�ȡ�������ݣ��������ݴ��ݵ���ɫ�������С�
��������Ƭ���У��������ö����������飬Ȼ��ʹ��glVertexAttribPointer����ָ���������ԣ�������ֱ�ָ����λ�á���������ͷ����������ǵ�λ��ֵ�ֱ�Ϊ0��1��2����С�ֱ�Ϊ3��2��3���ֱ����ÿ������һ���м���������������ΪGL_FLOAT��������������ֱ��ǲ�����ƫ����������ָ���������������ֽ�����ƫ����ָ���Ǹö��������ڻ������е�ƫ���������ƫ������ʹ��offsetof��������ȡ
�ڴ˴���Ƭ���У���ʹ��glGenBuffers����һ���µ�EBO��Element Buffer Object��������󶨵���ǰ�������С�EBO�������洢����ͼ�ε������Ļ�����������Ч�ؼ������ظ����ݵĴ洢����Ⱦ���������ǿ��Խ�����������VAO�������Ա��������Ⱦʱ���ٻ��Ƽ���ͼ�Ρ����ʹ��glBufferData�����������ϴ���EBO�����������С�
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

// ������ʯͷ������
const int num = 200;
// ʯͷ��С����
float scaleMax = 0.7f;
// ʯͷ��С����
float scaleMin = 0.1f;
// �洢��ͬʯͷ��С������
float scale[num];
// ʯͷY��������
float yMax = 8.0f;
// ʯͷY��������
float yMin = 5.0f;
// �洢��ͬʯͷY���������
float y[num];
// ���δ�С����
float circleMax = 135.0f;
// ���δ�С����
float circleMin = 65.0f;
// �洢��ͬʯͷZ���������
float z[num];
// �洢��ͬʯͷX���������
float x[num];


void initializedGL(void) //run only once
{
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW ��̫ OK." << std::endl;
    }

    std::cout << "���� : ���Ӻơ������� " << std::endl;
    std::cout << "ѧ�� : ����202008010103��������202008010115��" << std::endl;

    sendDataToOpenGL();

    // ����ufo
    UFO aUFO;
    aUFO.position = glm::vec3(0.0f, 0.0f, 20.0f);
    aUFO.status = UFO::intact;
    aUFO.x_movement_range[0] = -15.0f;
    aUFO.x_movement_range[1] = 15.0f;
    aUFO.movement_magnitude = 0.3f;
    ufo_container.push_back(aUFO);
    
    //�������λ��
    cameraPos = glm::vec3(0.0f, 2.5f, -6.5f);
    //���ö�����ɫ����Ƭ����ɫ��
    shader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");

    /*
     ����OpenGL����Ȳ��Ժ��޳��湦�ܡ�
��Ȳ�����OpenGL��һ�ֻ�������Ⱦ������������������������ڵ���ϵ������Ȳ����У�OpenGL�������������ֵ��һ���Ǵ����������֮��ľ��룩�ж������Ƿ�ɼ���������Ȳ��Կ���ͨ�� glEnable(GL_DEPTH_TEST) ��ʵ�֡�
�޳�����OpenGL����һ����Ⱦ�������������Զ��ڳ�����ĳЩ�����Ⱦ���������������Ⱦ�ٶȡ��������ڳ����в��ɼ��ı��棬û�б�Ҫ����Ⱦ�����л������ǣ���Ϊ������Զ���ᱻ��������OpenGL�������޳������ͨ�� glEnable(GL_CULL_FACE) ��ʵ�֡�
    */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);



    //���λ�÷�ʯͷ
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
    //���ù�Դ��λ�ú���ɫ�ȹ�����Ϣ
    //��Դ��ɫ
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
    glm::vec3 ambientColor = lightColor * glm::vec3(0.05f);

    //�����Դ��Ϣ
    shader.setVec3("dirLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
    shader.setVec3("dirLight.ambient", ambientColor);
    shader.setVec3("dirLight.diffuse", diffuseColor);
    shader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setFloat("dirLight.intensity", intensity);

    //���Դ��Ϣ
    shader.setVec3("pointLight.position", glm::vec3(15.0f, 7.0f, 35.0f));
    shader.setVec3("pointLight.ambient", ambientColor);
    shader.setVec3("pointLight.diffuse", diffuseColor);
    shader.setVec3("pointLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setFloat("pointLight.constant", 0.1f);
    shader.setFloat("pointLight.linear", 0.0014f);
    shader.setFloat("pointLight.quadratic", 0.00032f);

    //���ñ任����
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    //����͸��ͶӰ
    projectionMatrix = glm::perspective(glm::radians(75.0f), 1.0f, 0.1f, 500.0f);
    shader.setMat4("projectionMatrix", projectionMatrix);

    //0.05���¼����
    double crntTime = glfwGetTime();
    if (crntTime - prevTime >= 0.05) {
        //����ת
        rotation += 1.0f;
        prevTime = crntTime;
        
        /*
        ���У�ufo_container �ǰ��������� UFO ���������������ÿ�� UFO������λ�� x �������� movement_magnitude ���ӻ���٣�
        Ȼ����Խ��������˶���Χ�ı߽�ʱ��movement_magnitude �ͻ�ȡ�෴�����Ӷ��� UFO �����˶�������һ����
        ���� UFO ����ͻ������Ǹ��Ե��˶���Χ�������˶����Ӷ������˳�������ʵ�С�
        */
        for (std::vector<UFO>::iterator iter = ufo_container.begin(); iter != ufo_container.end(); iter++) {
            if (iter->position.x >= iter->x_movement_range[0] && iter->position.x <= iter->x_movement_range[1]) {
                
                iter->position.x += iter->movement_magnitude;//move forward on the track
            } else {
                iter->movement_magnitude *= -1.0f;
                iter->position.x += iter->movement_magnitude;//move backward on the track
            }
        }
        
        
        //��������
        if (keyboardCtl.LEFT)
            x_press_num += 1.0f;
        if (keyboardCtl.RIGHT)
            x_press_num -= 1.0f;
        if (keyboardCtl.UP)
            z_press_num += 1.0f;
        if (keyboardCtl.DOWN)
            z_press_num -= 1.0f;
        double xpos, ypos;//���λ��
        glfwGetCursorPos(window, &xpos, &ypos);
        rotation2 = (xpos-400) / 800 * 360;//rotation angle for spacecraft
    }
    
    for (it = rocketPos.begin();it != rocketPos.end();it++) {
        glm::mat4 modelMatrixR = *it;
        modelMatrixR = glm::translate(modelMatrixR, glm::vec3(0.0f, 0.0f, 0.5f * 800.0f));
        std::replace(rocketPos.begin(), rocketPos.end(), *it, modelMatrixR);
    }
    

    //����������ɴ���صľ�ֹ�����
    viewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    viewMatrix = glm::rotate(viewMatrix, glm::radians(rotation2), glm::vec3(0.0f, 0.5f, 0.0f));
    viewMatrix = glm::translate(viewMatrix, glm::vec3(-x_delta * x_press_num , 0.0f, -z_delta * z_press_num));
    //���͸���ɫ��
    shader.setVec3("eyePositionWorld", cameraPos);
    shader.setMat4("viewMatrix", viewMatrix);
    shader.setInt("normalMapping_flag", 0);//Ĭ�Ͻ��÷�����ͼ
    shader.setInt("light_flag", 1);

    

    //ƽ�ƺ���ת
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_delta * x_press_num, 0.0f, z_delta * z_press_num));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
    

    /*
    ��δ�����������һ��ģ�����ŵ���С�ĳߴ磬�Ա���õ�������ӽ�����ʾ��
    ������˵���� space Ϊ��ʱ����ģ�ͱ任���� modelMatrix ��Ӧ��һ�����ű任����Сģ�͵Ĵ�С��
    �����ñ任��ľ���ѹ�� rocketPos ������Ȼ�� space ��Ϊ�٣��Ա��������һ��ѭ��ʱ�ٴ�ִ����δ��롣
    ��󣬽�ԭʼ�����е����ű任��ԭ���Ա��⽫�������Ӧ�õ�����������Ⱦ�С�
    */
    if (space) {
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f / 1200.0f));
        rocketPos.push_back(modelMatrix);
        space = false;
        modelMatrix = glm::scale(modelMatrix, glm::vec3(800.0f));
    }
    
    /*
    ��ģ�;��� modelMatrix ������������ 1.0f / 400.0f �������Ų�����ʹ��ģ�ͱ�Ϊԭ���� 1/400 ��С��ʹ�� glm::scale �������в���
    �����ź�ľ��� modelMatrix ���ݸ���ɫ�� shader �Ը���ģ�ͱ任����ʹ�� shader.setMat4 �������в���
    ������ texture[0] �󶨵�����Ԫ 0 �ϣ�ʹ�� texture[0].bind �������в���
    ���������������ֵ myTextureSampler0 Ϊ 0��ʹ�� shader.setInt �������в���
    �󶨶������� vao[0]��ʹ�� glBindVertexArray �������в���
    ���� glDrawElements �������л��Ʋ���������ʹ�� GL_TRIANGLES ����ģʽ��������ģ���������� spacecraft.indices ��ƫ�������� 0
    ��󣬽�ģ�;��� modelMatrix ����˳�������ת��ƽ�Ʋ��������ȣ�������ת�Ƕ� rotation2 �� y ����תһ���Ƕȣ�
    ʹ�� glm::rotate �������в�����Ȼ�󣬸��ݱ任�� (-x_delta * x_press_num, 0.0f, -z_delta * z_press_num) ����ƽ�Ʋ�����ʹ�� glm::translate �������в���
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
    ͨ��ѭ������������ģ�͵�λ����Ϣ��ʹ�õ����� it ���в���
    ��ȡ��ǰ���ģ�͵�λ������ rocketLocVec�������ģ�ͱ任����ĵ�����������ʹ�� glm::vec3 �������в���
    �����ģ�͵����� texture[6] �󶨵�����Ԫ 0 �ϣ�ʹ�� texture[6].bind �������в���
    ���������������ֵ myTextureSampler0 Ϊ 0��ʹ�� shader.setInt �������в���
    �󶨻��ģ�͵Ķ������� vao[5]��ʹ�� glBindVertexArray �������в���
    ���ݵ�ǰ���ģ�͵ı任���� *it ����ģ�ͱ任�����ֵ��ʹ�� shader.setMat4 �������в���
    ʹ�� glDrawElements �������л��Ʋ���������ʹ�� GL_TRIANGLES ����ģʽ�������ݻ��ģ�͵��������� rocket.indices ��ƫ�������� 0����ɻ��ģ�͵Ļ��Ʋ���
    ������ͨ��ѭ������������ģ�͵�λ����Ϣ��������Ӧ������Ͷ������飬���ԶԶ�����ģ�ͽ��л��ƣ���ɻ����Ϸ��һ֡����Ⱦ��
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
    
    //����ʯͷ
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
framebuffer_size_callback ���ڼ������ڴ�С�ı仯�������ڴ�С�����仯ʱ��OpenGL �ӿ�Ҳ��Ҫ���µ�����glViewport �����������������ӿڴ�С���������Ϊ���½ǵ�λ�úͿ�ȸ߶ȡ�
mouse_button_callback ���ڼ�����갴ť��״̬������갴ť״̬�����仯ʱ�����Ը�����Ҫ������Ӧ����
cursor_position_callback ���ڼ��������λ�õı仯�������λ�÷����仯ʱ�����Ը�����Ҫ������Ӧ����
scroll_callback ���ڼ��������ֵĹ����������ֹ���ʱ�����Ը�����Ҫ������Ӧ����
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    cout << "��갴��" << endl;
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    cout << "������" << endl;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cout << "���ֶ���" << endl;
}



/*
������ ESCAPE ��ʱ������ glfwSetWindowShouldClose �����رմ���
������ LEFT ��ʱ���� keyboardCtl �ṹ���е� LEFT ��־��Ϊ�棬��ʾ��������ڱ�����
���ͷ� LEFT ��ʱ������Ӧ�� keyboardCtl �ṹ���еı�־��Ϊ�٣���ʾ������Ѿ����ͷ�
������ RIGHT��UP��DOWN ��ʱ��Ҳͬ���������ƵĴ���
������ W ��ʱ�������Դǿ����ȻС�� 1.0��������ǿ�� 0.1
������ S ��ʱ�������Դǿ����Ȼ���� 0.0����С��ǿ�� 0.1
������ SPACE ��ʱ���� space ��־��Ϊ true����ʾ��ʱ��Ҫִ��һ��ģ�����Ų���
������ R ��ʱ���������е� ufo_container �е� UFO ���󣬽����ǵ�״̬��Ϊ��ʼ״̬ intact���������ǵ�λ������Ϊ (0.0f, 0.0f, 20.0f)
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
    // ��ʼ�� GLFW ��

    //�涨�汾�ţ��涨����ģʽ�����ݡ�
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // ���� GLFW �⣬�������� OpenGL �汾Ϊ 3.3 ��ʹ�ú���(profiler)ģʽ

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "wzh&llx's final Graphics work", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;   // �����ڴ���ʧ�ܣ������������Ϣ
        glfwTerminate();
        return -1;
    }
    // ��������

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    // ʹ���ڵ�������Ϊ��ǰ������

    /*register callback functions*/
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // ע�Ỻ���С�ص�����
    glfwSetKeyCallback(window, key_callback);  // ע����̻ص�����
    glfwSetScrollCallback(window, scroll_callback);  // ע������ص�����
    glfwSetCursorPosCallback(window, cursor_position_callback);  // ע����λ�ûص�����
    glfwSetMouseButtonCallback(window, mouse_button_callback);  // ע����갴���ص�����
    // ע��ص�����

    initializedGL();   // �Զ����ʼ������


    /*
    while ѭ������ͨ������ paintGL() ��Ⱦ����������ÿ����Ⱦ���̽�����ʹ�� glfwSwapBuffers() ��������ǰ�󻺳������Ӷ���ʾ����Ⱦ�����
    ���⣬��δ���Ҳע����һЩ�ص�������
    */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */   // ��Ⱦ����
        paintGL();  // �Զ�����Ⱦ����

        /* Swap front and back buffers */
        glfwSwapBuffers(window);   // ����ǰ�󻺳���

        /* Poll for and process events */
        glfwPollEvents();   // �����¼�
    }

    glfwTerminate();  // �ͷ� GLFW ��Դ

    return 0;
}