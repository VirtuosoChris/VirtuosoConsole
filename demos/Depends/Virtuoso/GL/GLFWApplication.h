//
//  GLFWApplication.h
//  VirtuosoConsole
//
//  Created by Mac User on 8/21/20.
//

#ifndef GLFWApplication_h
#define GLFWApplication_h

// base class
struct GLFWApplication
{
    GLFWwindow* window;
    
    GLint currentWidth = 1024;
    GLint currentHeight = 768;
    
    GLint windowedWidth = 1024;
    GLint windowedHeight = 768;
    GLint windowLocX = 0;
    GLint windowLocY = 0;
    GLint contextMajor = 4;
    GLint contextMinor = 1;
    const bool vsync = true;
    bool fullscreen = false;
    bool debugContext = false;      // should be immutable
    
    std::string windowTitle = "";
    
    operator bool() const
    {
        return window;
    }
    
    struct FpsCounter
    {
    protected:
        double intervalStart = 0;
        std::size_t frames = 0;
        double fps = 0.0;
        bool _newValue;
    public:
        
        double intervalLength = 1.0f; // in seconds
        
        double value()
        {
            _newValue = false;
            return fps;
        }
        
        bool newValue()const{return _newValue;}
        
        void endFrame(double newTime)
        {
            frames++;

            if (newTime - intervalStart > intervalLength)
            {
                _newValue = true;
                fps = frames / intervalLength;
                intervalStart = newTime;
                frames = 0;
            }
        }
    };
                
    FpsCounter fpsCounter;
    
    GLFWApplication()
    {
        auto monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, contextMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, contextMinor);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,  debugContext);
         
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        
        if (!fullscreen)
        {
            window = glfwCreateWindow(windowedWidth, windowedHeight, windowTitle.c_str(), nullptr, NULL);
        }
        else
        {
            window = glfwCreateWindow(mode->width, mode->height, windowTitle.c_str(), monitor, NULL);
        }
        
        if (window)
        {
            glfwGetWindowPos(window, &windowLocX, &windowLocY);
            glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
            
            // callbacks
            glfwSetWindowSizeCallback(window, GLFWApplication::window_size_callback);
            glfwSetKeyCallback(window, GLFWApplication::key_callback);
            glfwSetMouseButtonCallback(window, GLFWApplication::mouse_button_callback);
            glfwSetCursorPosCallback(window, GLFWApplication::cursor_position_callback);
            glfwSetScrollCallback(window, GLFWApplication::scroll_callback);
            glfwSetWindowPosCallback(window, GLFWApplication::window_pos_callback);
            
            glfwSetWindowUserPointer(window, this); // associate this app object instance with the glfw window handle
            
            makeCurrent();
        }
    }
    
    inline double getTime() const
    {
        return glfwGetTime();
    }
    
    static GLFWApplication* application(GLFWwindow* window)
    {
        return static_cast<GLFWApplication*>(glfwGetWindowUserPointer(window));
    }
    
    static void window_pos_callback(GLFWwindow* window, int xpos, int ypos)
    {
        auto app = application(window);
        app->windowLocX = xpos;
        app->windowLocY = ypos;
        
        glfwGetWindowPos(window, &app->windowLocX, &app->windowLocY);
    }
    
    static void window_size_callback(GLFWwindow* window, int width, int height)
    {
        std::clog << "GLFW Window size callback "<< width<< " " << height <<std::endl;

        auto app = application(window);

        if (!app->fullscreen)
        {
            app->windowedWidth = width;
            app->windowedHeight = height;
        }
        
        app->currentWidth = width;
        app->currentHeight = height;
    }
    
    static void error_callback(int error, const char* description)
    {
        std::cerr<<"GLFW Error: " << description << std::endl;
    }
    
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
    {
    }

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
    }
    
    virtual void render()
    {
    }
    
    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            render();

            glfwSwapBuffers(window);

            fpsCounter.endFrame(getTime());
            
            glfwPollEvents();
        }
    }
    
    void makeCurrent()
    {
        assert(window);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(vsync ? 1 : 0); // active on context
    }
};


#endif /* GLFWApplication_h */
