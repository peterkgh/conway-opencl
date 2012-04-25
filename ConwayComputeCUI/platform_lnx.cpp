#include "stdafx.h"

#ifdef __linux__

unsigned int *redraw_buffer;

pthread_mutex_t mutex;

int window_id;

struct dispatcher_context *global_c;

int main (int argc, const char * argv[])
{
    return start_simulation();
}

unsigned long long
get_time(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void display(void)
{    
    glClear(GL_COLOR_BUFFER_BIT);
    
    pthread_mutex_lock(&mutex);
    
    for (int i = 0; i < (global_c->win_height * (global_c->columns / 8)) / 4; i++)
    {
        redraw_buffer[i] = ((redraw_buffer[i] & 0xFF000000) >> 24) |
        ((redraw_buffer[i] & 0x00FF0000) >> 8) |
        ((redraw_buffer[i] & 0x0000FF00) << 8) |
        ((redraw_buffer[i] & 0x000000FF) << 24);
    }
    
    glBitmap(global_c->columns, global_c->win_height, 400, 300, 0, 0, (const GLubyte*)redraw_buffer);
    
    glFlush();
    
    pthread_mutex_unlock(&mutex);
}

void idler(void)
{
}

bool initialize_window(void)
{
    char *argv = {0};
    int argc = 0;
    
	glutInit(&argc, &argv);
    
    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(0, 0);
    
	window_id = glutCreateWindow("LifeSim - OpenCL Accelerated");
    
    glutDisplayFunc(display);
    glutIdleFunc(idler);
    
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glColor3f(0, 0, 0);
    glRasterPos2f(0, 0);
    
    glutShowWindow();
    
    return true;
}

void update_gui(unsigned int *buff, size_t width, size_t height)
{
	redraw_buffer = buff;
    
    if (height > glutGet(GLUT_SCREEN_HEIGHT))    
        global_c->win_height = glutGet(GLUT_SCREEN_HEIGHT);
    else
        global_c->win_height = height;
    
    pthread_mutex_unlock(&mutex);
    
    glutReshapeWindow(global_c->columns, global_c->win_height);
    
    glutPostRedisplay();
    
    pthread_mutex_lock(&mutex);
    
    redraw_buffer = NULL;
}

int start_dispatcher(struct dispatcher_context *c)
{
	pthread_attr_t attr;
	pthread_t tid;
	int err;

    global_c = c;
    
    if (window_id)
    {
        //Spawn a thread so we can service the GUI here
        err = pthread_attr_init(&attr);
        if (err) return err;
        
        err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (err) return err;
        
        err = pthread_mutex_init(&mutex, NULL);
        if (err) return err;
        
        err = pthread_mutex_lock(&mutex);
        if (err) return err;
        
        err = pthread_create(&tid, &attr, dispatcher, c);
        if (err) return err;
        
        glutMainLoop();
    }
    else 
    {
        //Jump directly to the dispatcher
        dispatcher(c);
    }
    
    
	return 0;
}

#endif