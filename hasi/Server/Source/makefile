CC=g++
CFLAGS=-c -Wno-unknown-warning-option -Wno-psabi -Wall -std=c++0x -Wunreachable-code -Werror
LIBS=-ldl -lpthread -pthread
INCLUDES=-I.
SRC = main.cpp \
      HASConfig.cpp Has.cpp HASBasics.cpp HASBus.cpp \
      HASMember.cpp other-devices/SysInfo.cpp \
      WebServices/HTTPRequest.cpp WebServices/WeatherWebServiceDevice.cpp \
      WebServices/WebServiceDevice.cpp WebServices/WebServiceBus.cpp \
      WebServices/JSON.cpp WebServices/WeatherForecast.cpp\
      Network/HTTPServer.cpp \
      Network/RemoteListener.cpp Network/RemotePoolThread.cpp Network/HASIRemote.cpp\
      Network/NetworkDevice.cpp Network/NetworkServer.cpp \
      Network/NetworkInputServer.cpp Network/NetworkInputOutputClient.cpp\
      Network/NetworkInputOutputServer.cpp Network/NetworkBus.cpp \
      Network/HarmonyHubControl.cpp Network/HarmonyHubDevice.cpp \
      Script/ScriptExecuteCommand.cpp Script/ScriptExecuteManager.cpp \
      Script/CommandLine.cpp Script/CompoundExpression.cpp Script/Operator.cpp \
      Script/Primary.cpp Script/ExpressionParser.cpp Script/Value.cpp \
      Script/Variable.cpp Script/ClockManager.cpp Script/StopWatchManager.cpp \
      Script/ActivationCommand.cpp Script/ActivationManager.cpp \
      Script/TimerManager.cpp Script/RandomManager.cpp \
      Script/PulseManager.cpp Script/StateManager.cpp Script/OutputCommand.cpp Script/StateCommand.cpp \
      Script/PulseCommand.cpp Script/StopWatchCommand.cpp Script/Command.cpp Script/ExpressionCommand.cpp \
      Script/TimerCommand.cpp Script/ParserTools.cpp Script/CommandLooper.cpp Script/VarAssignments.cpp \
      Tools/Sockets.cpp Tools/AES.cpp Tools/Base64.cpp \
      Tools/SmallImage.cpp Tools/FilePlotter.cpp Tools/MailReporter.cpp \
      Tools/Threads.cpp Tools/SysTools.cpp Tools/MailPlotter.cpp \
      Tools/KeyValueFileParser.cpp Tools/Timer.cpp \
      Tools/DebugOutHandler.cpp\
      Tools/DebugOut/MultiplexOut.cpp Tools/DebugOut/ConsoleOut.cpp Tools/DebugOut/AbstrDebugOut.cpp \
      Tools/DebugOut/TextfileOut.cpp Tools/DebugOut/HTMLFileOut.cpp \
      I2C/I2CBase.cpp I2C/I2CBus.cpp I2C/I2CBusManager.cpp I2C/TCA9548ABusManager.cpp \
      I2C/OLED.cpp I2C/MB1242.cpp I2C/BV4627.cpp I2C/AI418S.cpp I2C/TMP006.cpp \
      I2C/ArduPiDAC.cpp I2C/HKAnalogPCF8591.cpp I2C/InputBlock.cpp I2C/RelayBlock.cpp I2C/ProxiVCNL4000.cpp \
      I2C/BlinkM.cpp I2C/I2CMember.cpp I2C/Mcp4725DAC.cpp I2C/ThermoHKLM75.cpp I2C/LuminosityTSL2561.cpp \
      I2C/BoschBMP.cpp I2C/BMP085_180Controller.cpp I2C/BMP280Controller.cpp  I2C/HIH-6130.cpp \
      Tools/Watchdog.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = has

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

pc: CFLAGS += -O3 -DNWDOG -DNI2C
pc: $(TARGET)

pcrelease: CFLAGS += -O3 -DNWDOG -DNI2C -DNDEBUG
pcrelease: $(TARGET)

svn:
	-svn up

$(TARGET): $(OBJ)
	$(CC) $(INCLUDES) $(LIBS) $^ -o $@

%.o: %.cpp
	$(CC) $(CMDFLAGS) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	-cp $(TARGET) $(TARGET)_old
	-rm -rf $(OBJ) $(TARGET) core

run:
	sudo ./$(TARGET)
