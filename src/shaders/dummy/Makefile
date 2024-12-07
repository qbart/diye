.PHONY: compile
compile:
	glslc -fshader-stage=vertex vert.glsl -o dummy.vert.spv
	glslc -fshader-stage=fragment frag.glsl -o dummy.frag.spv
	cp *.spv ../../../build