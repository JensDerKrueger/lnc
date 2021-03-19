#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <optional>

#include <GLApp.h>
#include <Client.h>
#include <GLTexture2D.h>
#include <FontRenderer.h>

#include <GLBuffer.h>
#include <GLArray.h>
#include <GLProgram.h>
#include <GLFramebuffer.h>
#include <Rand.h>
#include <Client.h>

#include <Tesselation.h>

class ChatConnection : public Client {
public:
  ChatConnection(const std::string& address, uint16_t port):
  Client{address, port , "", 5000}
  {
  }
  
  virtual void handleServerMessage(const std::string& message) override{
    try {
      const std::scoped_lock<std::mutex> lock(paintQueueMutex);
      Tokenizer t{message, char(1)};
      const std::string player  = base64url_decode(t.nextString());
      const std::string command = base64url_decode(t.nextString());
      paintQueue.push(command);
    } catch (const MessageException e) {
    }
  }

  void sendKeepAlivePing() {
    sendMessage("Ping");
  }

  std::optional<std::string> getPaintItem() {
    const std::scoped_lock<std::mutex> lock(paintQueueMutex);    
    if (paintQueue.empty()) return {};
    const std::string paintJob = paintQueue.front();
    paintQueue.pop();
    return paintJob;
  }

private:
  std::queue<std::string> paintQueue;
  std::mutex paintQueueMutex;
  
  bool executeCommand(char c, const std::string& player);
};




class ChatOfLife : public GLApp {
public:
  ChatOfLife();
  virtual ~ChatOfLife();
  
  virtual void animate(double animationTime) override;
  virtual void init() override;
  virtual void draw() override;
  virtual void keyboard(int key, int scancode, int action, int mods) override;
  virtual void mouseMove(double xPosition, double yPosition) override;
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override;
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) override;

private:
  std::vector<GLTexture2D> gridTextures{GLTexture2D{GL_NEAREST, GL_NEAREST},
                                        GLTexture2D{GL_NEAREST, GL_NEAREST}};
  GLBuffer vbFullScreenQuad{GL_ARRAY_BUFFER};
  GLBuffer ibFullScreenQuad{GL_ELEMENT_ARRAY_BUFFER};
  GLArray fullScreenQuadArray;
  GLProgram progFullscreenQuad{GLProgram::createFromFile("fullScreenQuadVS.glsl", "fullScreenQuadFS.glsl")};

  GLFramebuffer framebuffer;
  GLProgram progEvolve{GLProgram::createFromFile("fullScreenQuadVS.glsl", "evolveFS.glsl")};
  size_t current{0};
  int32_t paintState{0};
  float brushSize{1.0f};
  Vec2 paintPosition{-1.0f,-1.0f};
  double lastPingTime{0.0};

  // torus for visualization
  Tesselation torus{Tesselation::genTorus({0.0f, 0.0f, 0.0f}, 0.7f, 0.3f)};
  GLBuffer vbTorus{GL_ARRAY_BUFFER};
  GLBuffer nbTorus{GL_ARRAY_BUFFER};
  GLBuffer txTorus{GL_ARRAY_BUFFER};
  GLBuffer ibTorus{GL_ELEMENT_ARRAY_BUFFER};
  GLArray torusArray;
  GLProgram progTorus{GLProgram::createFromFile("visualizeVS.glsl", "visualizeFS.glsl")};

  bool drawTorus{false};
  bool paused{false};
  
  Mat4 mvp;
  Mat4 m;
  Vec3 lightPos;

  ChatConnection chatConnection;
  
  
  void randomizeGrid();
  void paintBitVector(std::vector<std::vector<uint8_t>>& bits, uint32_t gridX, uint32_t gridY, uint8_t direction, uint8_t value);
  std::vector<std::vector<uint8_t>> calcRawBitsFromMsg(const std::string &msg);
  std::pair<uint32_t, uint32_t> calcPositionFromMsg(const std::string &msg);
  uint8_t calcPatternTypeFromMsg(const std::string& msg);
  std::vector<std::vector<uint8_t>> calcLifeFormFromMsg(const std::string& msg);
  uint8_t calcDirectionFromMsg(const std::string& msg);
  void clearGrid();
  void paintPatternByMsg(const std::string &msg);

};
