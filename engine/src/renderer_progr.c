/*
	This file is part of SHMUP.

    SHMUP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SHMUP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SHMUP.  If not, see <http://www.gnu.org/licenses/>.
*/    
/*
 *  r_progr_renderer.c
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */


#include "renderer_progr.h"

#include "target.h"
#if defined (SHMUP_TARGET_WINDOWS) || defined (SHMUP_TARGET_MACOSX) || defined (SHMUP_TARGET_LINUX)
void initProgrRenderer(renderer_t* renderer){ Log_Printf("Shader renderer is not implemented.\n");exit(0);}
#else

#include "config.h"

#include "camera.h"
#include "filesystem.h"
#include "world.h"
#include "stats.h"
#include "config.h"
#include "player.h"
#include "enemy.h"

#if defined(ANDROID)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>	
#else
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#endif


matrix_t projectionMatrix;
matrix_t modelViewMatrix;
matrix_t modelViewProjectionMatrix;


typedef struct simple_shader_t {
	GLuint prog;
	GLuint vars[32];
	uchar props;
} shader_prog_t;


int lastId;
int lastBumpId;
int lastSpecId;


shader_prog_t shaders[8];
#define UNIFIED_LIGHT_SHADER 0
#define STRING_RENDER_SHADER 1
#define UNIFIED_LIGHT_SHADOW_SHADER 2
#define SHADOW_GENERATOR_SHADER 3

#define SHADER_MVT_MATRIX 0
#define SHADER_ATT_VERTEX 1
#define SHADER_ATT_NORMAL 2
#define SHADER_ATT_UV 3
#define SHADER_ATT_TANGENT 4
#define SHADER_TEXT_COLOR_SAMPLER 5
#define SHADER_TEXT_BUMP_SAMPLER 6
#define SHADER_UNI_LIGHT_POS 7
#define SHADER_UNI_LIGHT_COL_AMBIENT 8
#define SHADER_UNI_LIGHT_COL_DIFFUSE 9
#define SHADER_UNI_MV_MATRIX 10
#define SHADER_LIGHTPOV_MVT_MATRIX 11
#define SHADER_TEXT_SHADOWMAP_SAMPLER 12
#define SHADER_UNI_CAMERA_POS 13
#define SHADER_UNI_LIGHT_COL_SPECULAR 14
#define SHADER_UNI_MATERIAL_SHININESS 15

#define SHADER_TEXT_SPEC_SAMPLER 16
#define SHADER_UNI_MAT_COL_SPECULAR 17

#define NUM_UBERSHADERS 256
shader_prog_t* ubershaders[NUM_UBERSHADERS];
char* uberShaderVertex = 0;
char* uberShaderFragment = 0;
char ubserShaderDefines[8][64] = 
{
	"#define BUMP_MAPPING\n",
	"#define SPEC_MAPPING\n",
	"#define DIFF_MAPPING\n",
	"\n",
	"\n",
	"\n",
	"#define FOG",
	"#define SHADO_MAPPING\n"
};

char ubserShaderUndefines[8][64] = 
{
	"#undef BUMP_MAPPING\n",
	"#undef SPEC_MAPPING\n",
	"#undef DIFF_MAPPING\n",
	"\n",
	"\n",
	"\n",
	"#undef FOG",
	"#undef SHADO_MAPPING\n"
};

uchar PROP_NULL = 0;

shader_prog_t* currentShader;

GLuint shadowFBOId;
GLuint shadowMapTextureId;

float shadowMapRation = 1.0f;

int SRC_UseShader(shader_prog_t* shader)
{
		
	if (shader == 0)
	{
		return 0;
		Log_Printf("Shader was null: WTF !\n");
	}
	
	//if (shader == currentShader)
	//	return 0;
		
	
	STATS_AddShaderSwitch();
	
	glUseProgram(shader->prog); 
	currentShader = shader;
	
	lastId = -1;
	lastBumpId = -1;
	lastSpecId = -1;
	
	return 1;
}




void CreateFBOandShadowMap()
{
	GLenum status;
	GLuint   depthRenderbuffer; 
	
	
	glGenFramebuffers(1, &shadowFBOId);
	glBindFramebuffer(GL_FRAMEBUFFER,shadowFBOId);
	
	
	
	glGenTextures(1, &shadowMapTextureId);
	glBindTexture(GL_TEXTURE_2D, shadowMapTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, renderWidth*shadowMapRation, renderHeight*shadowMapRation, 0, GL_RGB565, GL_UNSIGNED_SHORT_5_6_5, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderer.glBuffersDimensions[WIDTH]*shadowMapRation, renderer.glBuffersDimensions[HEIGHT]*shadowMapRation, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, shadowMapTextureId,0);
	glBindTexture(GL_TEXTURE_2D, -1);
	
	glGenRenderbuffers(1, &depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, renderer.glBuffersDimensions[WIDTH]*shadowMapRation, renderer.glBuffersDimensions[HEIGHT]*shadowMapRation); 
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer); 
	
	
	
	
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
	switch (status) {
		case GL_FRAMEBUFFER_COMPLETE:						Log_Printf("GL_FRAMEBUFFER_COMPLETE\n");break;
		case 0x8CDB:										Log_Printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n");break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			Log_Printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	Log_Printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:			Log_Printf("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");break;			
		case GL_FRAMEBUFFER_UNSUPPORTED:					Log_Printf("GL_FRAMEBUFFER_UNSUPPORTED\n");break;	
		default:											Log_Printf("Unknown issue (%x).\n",status);break;	
	}	

	
	
	//glBindFramebuffer(GL_FRAMEBUFFER,0);
}


GLuint LoadShader(const char *shaderSrcPath, GLenum type, uchar props) 
{ 
	GLuint shader; 
	GLint compiled; 
	GLchar* sources[9];
	uchar i;
	int j;
    filehandle_t* shaderFile;
	
	Log_Printf("Loading %s\n",shaderSrcPath);
	
	memset(sources, 0, 9 * sizeof(GLchar*));
	
	// Create the shader object 
	shader = glCreateShader(type); 
	

	
	if(shader == 0) 
	{
		Log_Printf("Failed to created GL shader for '%s'\n",shaderSrcPath);
		return 0; 
	}
	
	
	
	shaderFile = FS_OpenFile(shaderSrcPath,"rt");
	FS_UploadToRAM(shaderFile);

	if (!shaderFile)
	{
		Log_Printf("Could not load shader: %s\n",shaderSrcPath);
		return 0;
	}
	
	i= 1 ;
	j= 0 ;
	do {
		
		if ((i & props) == i)
			sources[j] = ubserShaderDefines[j];
		else 
			sources[j] = ubserShaderUndefines[j] ;
		

		
		i *= 2;
		j++ ;
	} while (j< 8) ;
	
	sources[8] = (GLchar*)shaderFile->ptrStart;
	
	//for(i=0;i<8;i++)
	//	Log_Printf("%s",sources[i]);
	
	// Load the shader source 
	glShaderSource(shader, 9, (const GLchar**)sources, NULL); 
	

	
	
	
	// Compile the shader 
	glCompileShader(shader); 
	// Check the compile status 
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	
	if(!compiled) 
	{ 
		GLint infoLen = 0; 
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen); 
		
		if(infoLen > 1) 
		{ 
			char* infoLog = malloc(sizeof(char) * infoLen); 
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog); 
			Log_Printf("Error processing '%s' compiling shader:\n%s\n",shaderSrcPath, infoLog); 
			free(infoLog); 
		} 
		glDeleteShader(shader); 
		return 0; 
	} 
	return shader; 
}

void LoadProgram(shader_prog_t* shaderProg,const char* vertexShaderPath, const char* fragmentShaderPath, uchar props)
{
	GLuint vertexShader; 
	GLuint fragmentShader; 
	
	GLint linked;
	

	
	//Load simple shader
	vertexShader = LoadShader(vertexShaderPath,GL_VERTEX_SHADER,props);
	fragmentShader = LoadShader(fragmentShaderPath,GL_FRAGMENT_SHADER,props);
	
	
	// Create the program object 
	shaderProg->prog = glCreateProgram(); 
	if(shaderProg->prog == 0) 
	{
		Log_Printf("Could not create GL program.");
		return ; 
	}
	
	glAttachShader(shaderProg->prog, vertexShader); 
	glAttachShader(shaderProg->prog, fragmentShader);
	
	
	// Link the program 
	glLinkProgram(shaderProg->prog); 
	
	// Check the link status 
	glGetProgramiv(shaderProg->prog, GL_LINK_STATUS, &linked); 
	if(!linked) 
	{ 
		GLint infoLen = 0; 
		glGetProgramiv(shaderProg->prog, GL_INFO_LOG_LENGTH, &infoLen); 
		
		if(infoLen > 1) 
		{ 
			char* infoLog = malloc(sizeof(char) * infoLen); 
			glGetProgramInfoLog(shaderProg->prog, infoLen, NULL, infoLog); 
			Log_Printf("Error linking program:\n%s\n", infoLog); 
			
			free(infoLog); 
		} 
		glDeleteProgram(shaderProg->prog); 
		return ; 
	} 
	
}

void SCR_CheckErrorsF(char* step, char* details)
{
	GLenum err = glGetError();
	switch (err) {
		case GL_INVALID_ENUM:Log_Printf("Error GL_INVALID_ENUM %s, %s\n", step,details); break;
		case GL_INVALID_VALUE:Log_Printf("Error GL_INVALID_VALUE  %s, %s\n", step,details); break;
		case GL_INVALID_OPERATION:Log_Printf("Error GL_INVALID_OPERATION  %s, %s\n", step,details); break;				
		case GL_OUT_OF_MEMORY:Log_Printf("Error GL_OUT_OF_MEMORY  %s, %s\n", step,details); break;			
		case GL_NO_ERROR: break;
		default : Log_Printf("Error UNKNOWN  %s, %s \n", step,details);break;
	}
}


void SRC_BindUberShader(uchar props)
{
	uchar res_props;
	
	res_props = props & renderer.props;

	
	if (!ubershaders[res_props])
	{
		currentShader = calloc(1,sizeof(shader_prog_t));
		currentShader->props = res_props;
		ubershaders[res_props] = currentShader ;
		
		//MATLIB_printProp(res_props);
		LoadProgram(currentShader,"data/shaders/v_uber.glsl","data/shaders/f_uber.glsl",res_props);
		SCR_CheckErrorsF("Uber shader loaded.", "no details");
		
		
		//Get all uniforms and attributes
		
		currentShader->vars[SHADER_MVT_MATRIX]				= glGetUniformLocation(currentShader->prog,"modelViewProjectionMatrix");
		currentShader->vars[SHADER_TEXT_COLOR_SAMPLER]		= glGetUniformLocation(currentShader->prog,"s_baseMap");
		currentShader->vars[SHADER_TEXT_BUMP_SAMPLER]		= glGetUniformLocation(currentShader->prog,"s_bumpMap");	
		currentShader->vars[SHADER_UNI_LIGHT_POS]			= glGetUniformLocation(currentShader->prog,"lightPosition");
		currentShader->vars[SHADER_UNI_LIGHT_COL_AMBIENT]	= glGetUniformLocation(currentShader->prog,"lightColorAmbient");
		currentShader->vars[SHADER_UNI_LIGHT_COL_DIFFUSE]	= glGetUniformLocation(currentShader->prog,"lightColorDiffuse");
		currentShader->vars[SHADER_ATT_VERTEX]				= glGetAttribLocation(currentShader->prog,"a_vertex");
		currentShader->vars[SHADER_ATT_NORMAL]				= glGetAttribLocation(currentShader->prog,"a_normal");
		currentShader->vars[SHADER_ATT_UV]					= glGetAttribLocation(currentShader->prog,"a_texcoord0");
		currentShader->vars[SHADER_ATT_TANGENT]				= glGetAttribLocation(currentShader->prog,"a_tangent");
		currentShader->vars[SHADER_LIGHTPOV_MVT_MATRIX]		= glGetUniformLocation(currentShader->prog,"lightPOVPVMMatrix");
		currentShader->vars[SHADER_TEXT_SHADOWMAP_SAMPLER]	= glGetUniformLocation(currentShader->prog,"s_shadowpMap");
		currentShader->vars[SHADER_UNI_CAMERA_POS]			= glGetUniformLocation(currentShader->prog,"cameraPosition");
		currentShader->vars[SHADER_UNI_LIGHT_COL_SPECULAR]	= glGetUniformLocation(currentShader->prog,"lightColorSpecular");
		currentShader->vars[SHADER_UNI_MATERIAL_SHININESS]	= glGetUniformLocation(currentShader->prog,"materialShininess");
		currentShader->vars[SHADER_TEXT_SPEC_SAMPLER]		= glGetUniformLocation(currentShader->prog,"s_specularMap");
		currentShader->vars[SHADER_UNI_MAT_COL_SPECULAR]	= glGetUniformLocation(currentShader->prog,"matColorSpecular");
		SCR_CheckErrorsF("Ubershader attribute binded.", "no details");
	}
	
	if (SRC_UseShader(ubershaders[res_props]) )
	{
		if ((currentShader->props & PROP_SHADOW) == PROP_SHADOW)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D,shadowMapTextureId);
			glUniform1i ( currentShader->vars[SHADER_TEXT_SHADOWMAP_SAMPLER], 3 );
		}
	
		if ((currentShader->props & PROP_BUMP) == PROP_BUMP)
			glEnableVertexAttribArray(currentShader->vars[SHADER_ATT_TANGENT] );
		else
			glDisableVertexAttribArray(currentShader->vars[SHADER_ATT_TANGENT]);

		glEnableVertexAttribArray(currentShader->vars[SHADER_ATT_UV]);
		glEnableVertexAttribArray(currentShader->vars[SHADER_ATT_VERTEX] );
		glEnableVertexAttribArray(currentShader->vars[SHADER_ATT_NORMAL] );
	

	
		//Setup light
		glUniform3fv(currentShader->vars[SHADER_UNI_LIGHT_COL_AMBIENT],1,light.ambient);
		glUniform3fv(currentShader->vars[SHADER_UNI_LIGHT_COL_DIFFUSE],1,light.diffuse);
		glUniform3fv(currentShader->vars[SHADER_UNI_LIGHT_COL_SPECULAR],1,light.specula);
	}
}



void LoadAllShaders(void)
{
	LoadProgram(&shaders[STRING_RENDER_SHADER], "data/shaders/v_text.glsl", "data/shaders/f_text.glsl", PROP_NULL);	
	shaders[STRING_RENDER_SHADER].vars[SHADER_ATT_VERTEX] = glGetAttribLocation(shaders[STRING_RENDER_SHADER].prog,"a_vertex");
	shaders[STRING_RENDER_SHADER].vars[SHADER_ATT_UV] = glGetAttribLocation(shaders[STRING_RENDER_SHADER].prog,"a_texcoord0");
	shaders[STRING_RENDER_SHADER].vars[SHADER_TEXT_COLOR_SAMPLER] = glGetUniformLocation(shaders[STRING_RENDER_SHADER].prog,"s_baseMap");
	shaders[STRING_RENDER_SHADER].vars[SHADER_MVT_MATRIX] = glGetUniformLocation(shaders[STRING_RENDER_SHADER].prog,"modelViewProjectionMatrix");
	
	LoadProgram(&shaders[SHADOW_GENERATOR_SHADER], "data/shaders/v_shadowMapGenerator.glsl", "data/shaders/f_shadowMapGenerator.glsl",PROP_NULL) ;	
	shaders[SHADOW_GENERATOR_SHADER].vars[SHADER_ATT_VERTEX] = glGetAttribLocation(shaders[SHADOW_GENERATOR_SHADER].prog,"a_vertex");
	shaders[SHADOW_GENERATOR_SHADER].vars[SHADER_MVT_MATRIX]= glGetUniformLocation(shaders[SHADOW_GENERATOR_SHADER].prog,"modelViewProjectionMatrix");

	memset(ubershaders, 0, NUM_UBERSHADERS *  sizeof(shader_prog_t*));
	//Uber shader instanciation will be loaded on the fly.
	
}



void Set3D(void)
{
	glClear ( GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	//glColor4f(1, 1, 1, 1);
	
	renderer.isBlending = 0;
}

void StopRendition(void)
{
	lastId = -1;
	lastBumpId = -1;
	lastSpecId = -1;
	
	currentShader = 0; 
}

void UpLoadTextureToGPU(texture_t* texture)
{
	int i,mipMapDiv;
	
	if (!texture || !texture->data || texture->textureId != 0)
		return;
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture->textureId);
	glBindTexture(GL_TEXTURE_2D, texture->textureId);
	

	
	if (texture->format == TEXTURE_GL_RGB ||texture->format == TEXTURE_GL_RGBA)
	{
		//glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		Log_Printf("Warning mipmap for %s were not generated due to no GL_GENERATE_MIPMAP support.\n",texture->path);
        
        if(texture->format == TEXTURE_GL_RGBA)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data[0]);
        }
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->data[0]);
        
		free(texture->data[0]);
		texture->data[0] = 0;
	}
	else
	{
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, texture->format, texture->width,texture-> height, 0, texture->dataLength[0], texture->data[0]);
		mipMapDiv = 2;
		for (i=1; i < texture->numMipmaps; i++,mipMapDiv*=2) 
		{
			glCompressedTexImage2D(GL_TEXTURE_2D, i, texture->format, texture->width/mipMapDiv,texture-> height/mipMapDiv, 0, texture->dataLength[i], texture->data[i]);
			free(texture->data[i]);
		}
		
	}
	
	
	//Using mipMapping to reduce bandwidth consumption
	//Using mipMapping to reduce bandwidth consumption
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	//glTexParameterf(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_TRUE);
	
	
	
	//Testing mipmapping with nearest
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, DE_DEFAULT_FILTERING);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, DE_DEFAULT_FILTERING);	
	
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
	
	free(texture->dataLength);
	free(texture->data);
	
	if (texture->file != NULL)
		FS_CloseFile(texture->file);
		
		
	 
}

void SetTextures(material_t* material)
{
	unsigned int textureId;
	
	textureId = material->textures[TEXTURE_DIFFUSE].textureId ;
	
	if (lastId != textureId)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,textureId);
		glUniform1i ( currentShader->vars[SHADER_TEXT_COLOR_SAMPLER], 0 );
		STATS_AddTexSwitch();
		lastId = textureId;
	}
	
	if ( (currentShader->props & PROP_BUMP) == PROP_BUMP)
	{
		textureId = material->textures[TEXTURE_BUMP].textureId ;
		
		if (lastBumpId != textureId)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,textureId);
			glUniform1i ( currentShader->vars[SHADER_TEXT_BUMP_SAMPLER], 1 );
			lastBumpId = textureId;
		}
	}
	
	if ( (currentShader->props & PROP_SPEC) == PROP_SPEC)
	{
		textureId = material->textures[TEXTURE_SPECULAR].textureId ;
		
		if (lastSpecId != textureId)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D,textureId);
			glUniform1i ( currentShader->vars[SHADER_TEXT_SPEC_SAMPLER], 2 );
			lastSpecId = textureId;
		}
	}
	
}


void SetTexture(unsigned int textureId)
{
	if (lastId == textureId)
		return;
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,textureId);
	glUniform1i ( currentShader->vars[SHADER_TEXT_COLOR_SAMPLER], 0 );
	STATS_AddTexSwitch();
	lastId = textureId;
}

void SetupMD5forRendition(md5_mesh_t* mesh)
{

	if (!renderer.isRenderingShadow)
	{
		if (mesh->memLocation == MD5_MEMLOC_VRAM)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId);
			
             //This is very likely not 64bits friendly if the GPU copies stuff as it is presented.
			//glVertexAttribPointer(currentShader->vars[SHADER_ATT_VERTEX], 3, GL_FLOAT, GL_FALSE,  sizeof(vertex_t), (char*)NULL+VERTEX_T_DELTA_TO_POS);
			//glVertexAttribPointer(currentShader->vars[SHADER_ATT_NORMAL], 3, GL_SHORT, GL_TRUE,  sizeof(vertex_t), (char*)NULL+VERTEX_T_DELTA_TO_NORMAL);
			//glVertexAttribPointer(currentShader->vars[SHADER_ATT_UV], 2, GL_SHORT, GL_TRUE,  sizeof(vertex_t), (char*)NULL+VERTEX_T_DELTA_TO_TEXT);	
			//glVertexAttribPointer(currentShader->vars[SHADER_ATT_TANGENT], 3, GL_SHORT, GL_TRUE,  sizeof(vertex_t), (char*)NULL+VERTEX_T_DELTA_TO_TANGEN);	
			
            glVertexAttribPointer(currentShader->vars[SHADER_ATT_VERTEX], 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),  (char *)( (char *)(&mesh->vertexArray->pos   )  - ((char*)&mesh->vertexArray->pos)) );
			glVertexAttribPointer(currentShader->vars[SHADER_ATT_NORMAL],  3, GL_SHORT, GL_TRUE,  sizeof(vertex_t),  (char *)( (char *)(&mesh->vertexArray->normal)  - ((char*)&mesh->vertexArray->pos)) );
			glVertexAttribPointer(currentShader->vars[SHADER_ATT_UV],      2, GL_SHORT, GL_TRUE,  sizeof(vertex_t),  (char *)( (char *)(&mesh->vertexArray->text)    - ((char*)&mesh->vertexArray->pos)) );	
			glVertexAttribPointer(currentShader->vars[SHADER_ATT_TANGENT], 3, GL_SHORT, GL_TRUE,  sizeof(vertex_t),  (char *)( (char *)(&mesh->vertexArray->tangent) - ((char*)&mesh->vertexArray->pos)) );	
            
            
		}
		else 
		{
			glVertexAttribPointer(currentShader->vars[SHADER_ATT_NORMAL], 3, GL_SHORT, GL_TRUE,  sizeof(vertex_t), mesh->vertexArray->normal);
			glVertexAttribPointer(currentShader->vars[SHADER_ATT_TANGENT], 3, GL_SHORT, GL_TRUE,  sizeof(vertex_t), mesh->vertexArray->tangent);	
			glVertexAttribPointer(currentShader->vars[SHADER_ATT_UV], 2, GL_SHORT, GL_TRUE,  sizeof(vertex_t), mesh->vertexArray->text);			
		}
		
	}
	else 
	{
		if (mesh->memLocation == MD5_MEMLOC_VRAM)
			glVertexAttribPointer(currentShader->vars[SHADER_ATT_VERTEX], 3, GL_FLOAT, GL_FALSE,  sizeof(vertex_t), 0);
		else
			glVertexAttribPointer(currentShader->vars[SHADER_ATT_VERTEX], 3, GL_FLOAT, GL_FALSE,  sizeof(vertex_t), mesh->vertexArray->pos);
	}

	
	
}

void ComputeInvModelMatrix(matrix_t matrix, matrix_t dest)
{
	
	matrix_t invTranslation;
	matrix_t invRotation;
	
	
	invTranslation[0] = 1 ;	invTranslation[4] = 0 ;	invTranslation[8] = 0 ;		invTranslation[12] = -matrix[12] ;
	invTranslation[1] = 0 ;	invTranslation[5] = 1 ;	invTranslation[9] = 0 ;		invTranslation[13] = -matrix[13] ;
	invTranslation[2] = 0 ;	invTranslation[6] = 0 ;	invTranslation[10] = 1 ;	invTranslation[14] = -matrix[14] ;
	invTranslation[3] = 0 ;	invTranslation[7] = 0 ;	invTranslation[11] = 0 ;	invTranslation[15] = 1 ;
	
	invRotation[0] = matrix[0] ;		invRotation[4] = matrix[1] ;		invRotation[8] = matrix[2] ;		invRotation[12] = 0 ;
	invRotation[1] = matrix[4] ;		invRotation[5] = matrix[5] ;		invRotation[9] = matrix[6] ;		invRotation[13] = 0 ;
	invRotation[2] = matrix[8] ;		invRotation[6] = matrix[9] ;		invRotation[10] = matrix[10] ;		invRotation[14] = 0 ;
	invRotation[3] = 0 ;				invRotation[7] = 0 ;				invRotation[11] = 0 ;				invRotation[15] = 1 ;
	
	
	matrix_multiply(invRotation, invTranslation, dest);
}


matrix_t tmp;
matrix_t inv_modelMatrix;
vec4_t modelSpaceLightPos;
vec4_t modelSpaceCameraPos;

static void RenderEntity(entity_t* entity)
{

	
	SRC_BindUberShader(entity->material->prop);
	
	
	matrix_multiply(modelViewMatrix,entity->matrix, tmp);
	matrix_multiply(projectionMatrix,tmp, modelViewProjectionMatrix);
	glUniformMatrix4fv(currentShader->vars[SHADER_MVT_MATRIX]   ,1,GL_FALSE,modelViewProjectionMatrix);
	
	ComputeInvModelMatrix(entity->matrix, inv_modelMatrix);
	
	matrix_transform_vec4t(inv_modelMatrix, light.position, modelSpaceLightPos);
	glUniform3fv(currentShader->vars[SHADER_UNI_LIGHT_POS],1,modelSpaceLightPos);
	
	matrix_transform_vec4t(inv_modelMatrix, camera.position, modelSpaceCameraPos);
	glUniform3fv(currentShader->vars[SHADER_UNI_CAMERA_POS],1,modelSpaceCameraPos);
	
	
	
	if ((renderer.props & PROP_SHADOW) == PROP_SHADOW)
		glUniformMatrix4fv(currentShader->vars[SHADER_LIGHTPOV_MVT_MATRIX]   ,1,GL_FALSE,entity->cachePVMShadow);
	
	glUniform1f(currentShader->vars[SHADER_UNI_MATERIAL_SHININESS], entity->material->shininess);
	glUniform3fv(currentShader->vars[SHADER_UNI_MAT_COL_SPECULAR],1,entity->material->specularColor);
	
	SetTextures(entity->material);
	
	if (entity->material->hasAlpha )
	{
		if (!renderer.isBlending)
		{
			renderer.isBlending = 1;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			STATS_AddBlendingSwitch();
		}
	}
	else
	{
		if (renderer.isBlending)
		{
			renderer.isBlending = 0;
			glDisable(GL_BLEND);
			STATS_AddBlendingSwitch();
		}
	}
	
	
	
	SetupMD5forRendition(entity->model);
	
	if (entity->usage == ENT_PARTIAL_DRAW)
	{
		glDrawElements (GL_TRIANGLES, entity->numIndices, GL_UNSIGNED_SHORT, entity->indices);	
		if (!renderer.isRenderingShadow)
			STATS_AddTriangles(entity->numIndices/3);

	}
	else
	{
		glDrawElements (GL_TRIANGLES, entity->model->numIndices, GL_UNSIGNED_SHORT, entity->model->indices);
		if (!renderer.isRenderingShadow)
			STATS_AddTriangles(entity->model->numIndices/3);

	}
	
}

void SetupCamera(void)
{
	vec3_t vLookat;
	
	vectorAdd(camera.position,camera.forward,vLookat);
	
	gluLookAt(camera.position, vLookat, camera.up, modelViewMatrix);
	
	gluPerspective(camera.fov, camera.aspect,camera.zNear, camera.zFar, projectionMatrix);
	
}




void RenderEntities(void)
{
	int i;
	entity_t* entity;
	enemy_t* enemy;
	
	
	
	renderer.isRenderingShadow = ((renderer.props & PROP_SHADOW) == PROP_SHADOW) ;
	
	
	if (renderer.isRenderingShadow)
	{
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER,shadowFBOId);
		glClearColor(1.0,1.0,1.0,1.0);
		
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0,0.0,0.0,1.0);
		
		SRC_UseShader(&shaders[SHADOW_GENERATOR_SHADER]);
		glEnableVertexAttribArray(currentShader->vars[SHADER_ATT_VERTEX] );
		glViewport(0, 0, renderer.glBuffersDimensions[WIDTH]*shadowMapRation, renderer.glBuffersDimensions[HEIGHT]*shadowMapRation);
		
		//Setup perspective and camera
		gluPerspective(light.fov, camera.aspect,camera.zNear, camera.zFar, projectionMatrix);
		gluLookAt(light.position, light.lookAt, light.upVector, modelViewMatrix);
	
		entity = map;
		for(i=0; i < num_map_entities; i++,entity++)
		{		
			if (entity->numIndices == 0)
				continue;
			
			matrix_multiply(modelViewMatrix,entity->matrix, tmp);
			matrix_multiply(projectionMatrix,tmp, modelViewProjectionMatrix);
			glUniformMatrix4fv(currentShader->vars[SHADER_MVT_MATRIX]   ,1,GL_FALSE,modelViewProjectionMatrix);
			
			SetupMD5forRendition(entity->model);
			
			glDrawElements (GL_TRIANGLES, entity->numIndices, GL_UNSIGNED_SHORT, entity->indices);
			if (!renderer.isRenderingShadow)
				STATS_AddTriangles(entity->numIndices/3);
			
			//Also need to cache the PVM matrix in entity
			matrixCopy(modelViewProjectionMatrix,entity->cachePVMShadow);
		}
		
		//NOTE: Not rendering player because shadow will never be used in action phases
		
		glViewport(renderer.viewPortDimensions[VP_X],
				   renderer.viewPortDimensions[VP_Y],
				   renderer.viewPortDimensions[VP_WIDTH],
				   renderer.viewPortDimensions[VP_HEIGHT]);
		
		glCullFace(GL_BACK);
		
		renderer.isRenderingShadow = 0;
		
		glBindFramebuffer(GL_FRAMEBUFFER,renderer.mainFramebufferId);
	}
	
	
	
	//Setup perspective and camera
	SetupCamera();
	
	entity = map;
	
	glDisable(GL_CULL_FACE);
	for(i=0; i < num_map_entities; i++,entity++)
	{
		if (entity->numIndices == 0)
			continue;
		
		RenderEntity(entity);		
	}
	glEnable(GL_CULL_FACE);
	
	
	// Rendering players
	for (i=0 ; i < numPlayers; i++) 
	{
		entity = &players[i].entity ;
		
		RenderEntity(entity);
	}
	
	enemy = ENE_GetFirstEnemy();
	while (enemy != NULL) 
	{
		entity = &enemy->entity;
				
		RenderEntity(entity);
		
		enemy = enemy->next;
	} 
}

void RenderString(xf_colorless_sprite_t* vertices,ushort* indices, uint numIndices)
{	
	
	
	glVertexAttribPointer(currentShader->vars[SHADER_ATT_VERTEX], 2, GL_SHORT, GL_FALSE,  sizeof(xf_colorless_sprite_t), vertices->pos);
	glVertexAttribPointer(currentShader->vars[SHADER_ATT_UV], 2, GL_SHORT, GL_FALSE,  sizeof(xf_colorless_sprite_t), vertices->text);			

	glDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT,indices);
	

	STATS_AddTriangles(numIndices/3);
	
}

void SetupLighting(void)
{
	//vec3_t cameraSpaceLightPos;
	

	
	// Need to send the position in camera space, best practice is to use a normal matrix (transpose inverse) ....
	// But who fucking cares ?
//	matrix_multiplyVertexByMatrix(light.position,modelViewMatrix,cameraSpaceLightPos);
	
				
}


void Set2D(void)
{
	
	float t1,t2,t3;
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	
	t1 = -(SS_W + -SS_W)/ (float)(SS_W  - -SS_W);
	t2 = -(SS_H + -SS_H)/(float)(SS_H - -SS_H);
	t3 = -(1 +  -1)/(1 - -1);
	
	modelViewProjectionMatrix[0] = 2.0f/ (SS_W - - SS_W )	; modelViewProjectionMatrix[4] = 0								; modelViewProjectionMatrix[8] = 0			; modelViewProjectionMatrix[12] =t1 ;
	modelViewProjectionMatrix[1] = 0								; modelViewProjectionMatrix[5] = 2.0f/(SS_H - -SS_H); modelViewProjectionMatrix[9] = 0			; modelViewProjectionMatrix[13] = t2 ;
	modelViewProjectionMatrix[2] = 0								; modelViewProjectionMatrix[6] = 0									; modelViewProjectionMatrix[10] = -2.0f/(1 - -1); modelViewProjectionMatrix[14] = t3;
	modelViewProjectionMatrix[3] = 0								; modelViewProjectionMatrix[7] = 0									; modelViewProjectionMatrix[11] = 0			; modelViewProjectionMatrix[15] = 1  ;	
	
	SRC_UseShader(&shaders[STRING_RENDER_SHADER]);
	glEnableVertexAttribArray(currentShader->vars[SHADER_ATT_UV]);
	glEnableVertexAttribArray(currentShader->vars[SHADER_ATT_VERTEX] );
	//matrixLoadIdentity(modelViewProjectionMatrix);
	glUniformMatrix4fv(currentShader->vars[SHADER_MVT_MATRIX],1,GL_FALSE,modelViewProjectionMatrix);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//glBindFramebuffer(GL_FRAMEBUFFER,0);
	//SetTexture(2);
	
	/*
	//Draw sprite(textureId, vertices, indices);	
	SetTexture(1);
	glVertexAttribPointer(currentShader->vars[SHADER_ATT_VERTEX], 2, GL_SHORT, GL_FALSE,  sizeof(svertex_t), verticesSprite->pos);
	glVertexAttribPointer(currentShader->vars[SHADER_ATT_UV], 2, GL_FLOAT, GL_FALSE,  sizeof(svertex_t), verticesSprite->text);			
	//TODO CHANGE glDrawElements
	glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,indicesSprite);
	 */
}



void GetColorBuffer(uchar* data)
{
	glReadPixels(0,0,renderer.glBuffersDimensions[WIDTH],renderer.glBuffersDimensions[HEIGHT],GL_RGBA, GL_UNSIGNED_BYTE,data);
}

void RenderPlayersBullets(void)
{
	
	if (numPBulletsIndices == 0)
		return;
	
	SetTexture(bulletConfig.bulletTexture.textureId);
	
	
	glVertexAttribPointer(currentShader->vars[SHADER_ATT_VERTEX], 2, GL_SHORT, GL_FALSE,  sizeof(xf_colorless_sprite_t), pBulletVertices->pos);
	glVertexAttribPointer(currentShader->vars[SHADER_ATT_UV], 2, GL_SHORT, GL_TRUE,  sizeof(xf_colorless_sprite_t), pBulletVertices->text);			

	glDrawElements (GL_TRIANGLES, numPBulletsIndices, GL_UNSIGNED_SHORT,bulletIndices);
	 
	STATS_AddTriangles(numPBulletsIndices/3);
	
	
}



void UpLoadEntityToGPU(entity_t* entity)
{
	md5_mesh_t* mesh;
	
	if (entity == NULL || entity->model == NULL)
	{
		Log_Printf("Entity was NULL: No vertices to upload.\n");
		return;
	}
	
	if (entity->model->memLocation == MD5_MEMLOC_VRAM)
		return;		
	
	
	mesh = entity->model;
	
	glGenBuffers(1, &mesh->vboId);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId);
	glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(vertex_t), mesh->vertexArray, GL_STATIC_DRAW);
	
	
	free(mesh->vertexArray);
	
	mesh->memLocation = MD5_MEMLOC_VRAM;
}


void RenderFXSprites(void)
{
	Log_Printf("Not implemented (RenderFXSprites).!\n");
}

void DrawControls(void)
{
	Log_Printf("Not implemented (DrawControls) .!\n");
}


void FreeGPUTexture(texture_t* texture)
{
	glDeleteTextures(1, &texture->textureId);
	texture->textureId = 0;
}


uint UploadVerticesToGPU(void* vertices, uint mem_size)
{
	uint vboId;
	
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, mem_size, vertices, GL_STATIC_DRAW);
	
	return vboId;
}

void StartCleanFrame(void)
{
	//glClear(GL_COLOR_BUFFER_BIT );
	glEnable(GL_TEXTURE_2D);
}

void RenderColorlessSprites(xf_colorless_sprite_t* vertices, ushort numIndices,ushort* indices)
{
	Log_Printf("Not implemented (RenderColorlessSprites) .!\n");
}



void FadeScreen(float alpha)
{
	Log_Printf("Not implemented (FadeScreen) .!\n");
}

void SetMaterialTextureBlending(char modulate)
{
	Log_Printf("Not implemented (SetMaterialTextureBlending) .!\n");
	
}

void SetTransparency(float alpha)
{
	Log_Printf("Not implemented (SetTransparency) .!\n");
	
}



void FreeGPUBuffer(uint bufferId)
{
	glDeleteBuffers(1,&bufferId);
	
}


int supportedCompressionFormat;
int IsTextureCompressionSupported(int type){
    return supportedCompressionFormat & type;
}

void RefreshViewPort()
{
    glViewport(renderer.viewPortDimensions[VP_X],
			   renderer.viewPortDimensions[VP_Y], 
			   renderer.viewPortDimensions[VP_WIDTH], 
			   renderer.viewPortDimensions[VP_HEIGHT]);
}

void initProgrRenderer(renderer_t* renderer)
{
    char *extensionsList;
	/*
	int numAttVert;
	int numUnifVert, numUnifFrag;
	int numVary;
	*/
	
	SCR_CheckErrorsF("initProgrRenderer","438");
	
	renderer->type = GL_20_RENDERER ;
	
	renderer->props |= PROP_BUMP;
	renderer->props |= PROP_SPEC;
	renderer->props |= PROP_DIFF;
	renderer->props |= PROP_SHADOW;

	renderer->Set3D = Set3D;
	renderer->StopRendition = StopRendition;
//	renderer->PushMatrix = PushMatrix;
//	renderer->PopMatrix = PopMatrix;
	renderer->SetTexture = SetTexture;
	renderer->RenderEntities = RenderEntities;
	renderer->UpLoadTextureToGpu = UpLoadTextureToGPU;
	renderer->RenderPlayersBullets = RenderPlayersBullets ;
	renderer->UpLoadEntityToGPU = UpLoadEntityToGPU;
	renderer->Set2D = Set2D;
	renderer->RenderString = RenderString;
	renderer->GetColorBuffer = GetColorBuffer;
	
	renderer->RenderFXSprites = RenderFXSprites;
	renderer->DrawControls = DrawControls;
	
	renderer->FreeGPUTexture = FreeGPUTexture;
	renderer->FreeGPUBuffer = FreeGPUBuffer;
	
	renderer->UploadVerticesToGPU = UploadVerticesToGPU;
	renderer->StartCleanFrame= StartCleanFrame;
	renderer->RenderColorlessSprites = RenderColorlessSprites;
	renderer->FadeScreen = FadeScreen;
	renderer->SetMaterialTextureBlending = SetMaterialTextureBlending;
	renderer->SetTransparency = SetTransparency;
	renderer->IsTextureCompressionSupported = IsTextureCompressionSupported;
    renderer->RefreshViewPort = RefreshViewPort;
	
	//Clean up screen first
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	glViewport(renderer->viewPortDimensions[VP_X], 
			   renderer->viewPortDimensions[VP_Y], 
			   renderer->viewPortDimensions[VP_WIDTH], 
			   renderer->viewPortDimensions[VP_HEIGHT]);
	

	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glClearColor(0, 0, 0,1.0f);
	
	LoadAllShaders();
	
	//Create shadowMap and FBO
	CreateFBOandShadowMap();
	
    //We need to check what texture compression method is supported.
    extensionsList = (char *) glGetString(GL_EXTENSIONS);
    if (strstr(extensionsList,"GL_IMG_texture_compression_pvrtc"))
        supportedCompressionFormat |= TEXTURE_FORMAT_PVRTC ;
	
	SCR_CheckErrorsF("End of initProgrRenderer", "no details");

}


#endif