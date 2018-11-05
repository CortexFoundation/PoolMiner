#include <time.h>

#include "trimmer_cl.h"
namespace cuckoogpu {

#define DUCK_A_EDGES_NX (DUCK_A_EDGES * NX)
#define DUCK_B_EDGES (EDGES_B)
#define DUCK_B_EDGES_NX (DUCK_B_EDGES * NX)

    edgetrimmer::edgetrimmer(const trimparams _tp, cl_context context,
			     cl_command_queue commandQueue,
			     cl_program program) {
	this->context = context;
	this->commandQueue = commandQueue;
	this->program = program;
	indexesSize = NX * NY * sizeof (u32);
	tp = _tp;

	cl_int clResult;
	 dipkeys =
	    clCreateBuffer(this->context, CL_MEM_READ_WRITE,
			   sizeof (siphash_keys), NULL, &clResult);
	if (clResult != CL_SUCCESS) {
	    fprintf(stderr, "%s\n", getCLErrorStr(clResult));
	    return;
	}
	indexesE =
	    clCreateBuffer(context, CL_MEM_READ_WRITE, indexesSize, NULL,
			   &clResult);
	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "create buffer indexesE error : %d\n", clResult);
	    return;
	}
	indexesE2 =
	    clCreateBuffer(context, CL_MEM_READ_WRITE, indexesSize, NULL,
			   &clResult);
	if (clResult != CL_SUCCESS)
	{
	    printf("create buffer indexesE2 error : %d\n", clResult);
	    return;
	}

	recoveredges =
	    clCreateBuffer(context, CL_MEM_READ_WRITE,
			   sizeof (cl_uint2) * PROOFSIZE, NULL, &clResult);
	if (clResult != CL_SUCCESS)
	{
	    printf("create buffer recoveredges error:%d", clResult);
	    return;
	}

	sizeA = ROW_EDGES_A * NX * sizeof (cl_uint2);
	sizeB = ROW_EDGES_B * NX * sizeof (cl_uint2);

	const size_t bufferSize = sizeA + sizeB;
	fprintf(stderr, "bufferSize: %lu\n", bufferSize);
	bufferA =
	    clCreateBuffer(context, CL_MEM_READ_WRITE, sizeA, NULL, &clResult);
	if (clResult != CL_SUCCESS)
	{
	    printf("create buffer bufferA error : %d\n", clResult);
	    return;
	}
	bufferB =
	    clCreateBuffer(context, CL_MEM_READ_WRITE, sizeB, NULL, &clResult);
	bufferAB =
	    clCreateBuffer(context, CL_MEM_READ_WRITE, sizeA, NULL, &clResult);
    }

    u64 edgetrimmer::globalbytes() const {
	return (sizeA * 2 + sizeB) + 2 * indexesSize + sizeof (siphash_keys);
    }
    edgetrimmer::~edgetrimmer() {
	clReleaseMemObject(bufferA);
	clReleaseMemObject(bufferB);
	clReleaseMemObject(bufferAB);
	clReleaseMemObject(recoveredges);
	clReleaseMemObject(indexesE2);
	clReleaseMemObject(indexesE);
	clReleaseMemObject(dipkeys);
	releaseCommandQueue(commandQueue);
	releaseProgram(program);
	releaseContext(context);
    }

    u32 edgetrimmer::trim(uint32_t device) {
	int initV = 0;

	clFinish(commandQueue);

	size_t tmpSize = indexesSize;

	cl_int clResult;
	clResult =
	    clEnqueueFillBuffer(commandQueue, indexesE, &initV, sizeof (int), 0,
				tmpSize, 0, NULL, NULL);
	if (clResult != CL_SUCCESS)
	{
	    printf("fill buffer indexesE error: %d\n", clResult);
	    return 0;
	}
	clResult =
	    clEnqueueFillBuffer(commandQueue, indexesE2, &initV, sizeof (int),
				0, tmpSize, 0, NULL, NULL);
	if (clResult != CL_SUCCESS)
	{
	    printf("fill buffer indexesE2 error: %d\n", clResult);
	    return 0;
	}
	clResult =
	    clEnqueueWriteBuffer(commandQueue, dipkeys, CL_TRUE, 0,
				 sizeof (siphash_keys), &sipkeys, 0, NULL,
				 NULL);
	if (clResult != CL_SUCCESS)
	{
	    printf("fill buffer dipkeys error : %d\n", clResult);
	    return 0;
	}
	clFinish(commandQueue);

	size_t global_work_size[1];
	size_t local_work_size[1];
	global_work_size[0] = tp.genA.blocks * tp.genA.tpb;
	local_work_size[0] = tp.genA.tpb;
	cl_event event;
	int edges_a = EDGES_A;
	cl_kernel seedA_kernel = clCreateKernel(program, "SeedA", &clResult);
	clResult =
	    clSetKernelArg(seedA_kernel, 0, sizeof (cl_mem), (void *) &dipkeys);
	clResult =
	    clSetKernelArg(seedA_kernel, 1, sizeof (cl_mem),
			   (void *) &bufferAB);
	clResult =
	    clSetKernelArg(seedA_kernel, 2, sizeof (cl_mem),
			   (void *) &indexesE);
	clResult = clSetKernelArg(seedA_kernel, 3, sizeof (int), &edges_a);

	clResult =
	    clEnqueueNDRangeKernel(commandQueue, seedA_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "%s\n", getCLErrorStr(clResult));
	    return -1;
	}
	clFinish(commandQueue);

	u32 halfA0 = 0;
	u32 halfE0 = 0;
	u32 halfA = sizeA / 2;	///sizeof(cl_ulong4);
	u32 halfE = NX2 / 2;

	global_work_size[0] = tp.genB.blocks / 2 * tp.genB.tpb;
	local_work_size[0] = tp.genB.tpb;
	cl_kernel seedB_kernel = clCreateKernel(program, "SeedB", &clResult);
	clResult |=
	    clSetKernelArg(seedB_kernel, 0, sizeof (cl_mem), (void *) &dipkeys);
	clResult |=
	    clSetKernelArg(seedB_kernel, 1, sizeof (cl_mem),
			   (void *) &bufferAB);
	clResult |=
	    clSetKernelArg(seedB_kernel, 2, sizeof (cl_mem), (void *) &bufferA);
	clResult |=
	    clSetKernelArg(seedB_kernel, 3, sizeof (cl_mem),
			   (void *) &indexesE);
	clResult |=
	    clSetKernelArg(seedB_kernel, 4, sizeof (cl_mem),
			   (void *) &indexesE2);
	clResult |= clSetKernelArg(seedB_kernel, 5, sizeof (int), &edges_a);
	clResult |= clSetKernelArg(seedB_kernel, 6, sizeof (u32), &halfA0);
	clResult |= clSetKernelArg(seedB_kernel, 7, sizeof (u32), &halfE0);
	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "call SeedB , cl error : %s\n",
		    getCLErrorStr(clResult));
	    return -1;
	}

	clResult =
	    clEnqueueNDRangeKernel(commandQueue, seedB_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	clFinish(commandQueue);

	clResult |=
	    clSetKernelArg(seedB_kernel, 0, sizeof (cl_mem), (void *) &dipkeys);
	clResult |=
	    clSetKernelArg(seedB_kernel, 1, sizeof (cl_mem),
			   (void *) (&bufferAB));
	clResult |=
	    clSetKernelArg(seedB_kernel, 2, sizeof (cl_mem),
			   (void *) (&bufferA));
	clResult |=
	    clSetKernelArg(seedB_kernel, 3, sizeof (cl_mem),
			   (void *) (&indexesE));
	clResult |=
	    clSetKernelArg(seedB_kernel, 4, sizeof (cl_mem),
			   (void *) (&indexesE2));
	clResult |= clSetKernelArg(seedB_kernel, 5, sizeof (int), &edges_a);
	clResult |= clSetKernelArg(seedB_kernel, 6, sizeof (u32), &halfA);
	clResult |= clSetKernelArg(seedB_kernel, 7, sizeof (u32), &halfE);
	clResult |=
	    clEnqueueNDRangeKernel(commandQueue, seedB_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);

	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "call seedb error : %s\n", getCLErrorStr(clResult));
	    return -1;
	}
	clFinish(commandQueue);

	clEnqueueFillBuffer(commandQueue, indexesE, &initV, sizeof (int), 0,
			    tmpSize, 0, NULL, NULL);
	clFinish(commandQueue);

	cl_kernel round_kernel = clCreateKernel(program, "Round", &clResult);
	global_work_size[0] = tp.trim.blocks * tp.trim.tpb;
	local_work_size[0] = tp.trim.tpb;
	int constRound = 0;
	edges_a = EDGES_A;
	int edges_b = EDGES_B;
	clResult |= clSetKernelArg(round_kernel, 0, sizeof (int), &constRound);
	clResult |=
	    clSetKernelArg(round_kernel, 1, sizeof (cl_mem), (void *) &dipkeys);
	clResult |=
	    clSetKernelArg(round_kernel, 2, sizeof (cl_mem), (void *) &bufferA);
	clResult |=
	    clSetKernelArg(round_kernel, 3, sizeof (cl_mem), (void *) &bufferB);
	clResult |=
	    clSetKernelArg(round_kernel, 4, sizeof (cl_mem),
			   (void *) &indexesE2);
	clResult |=
	    clSetKernelArg(round_kernel, 5, sizeof (cl_mem),
			   (void *) &indexesE);
	clResult |= clSetKernelArg(round_kernel, 6, sizeof (int), &edges_a);
	clResult |= clSetKernelArg(round_kernel, 7, sizeof (int), &edges_b);
	clResult |=
	    clEnqueueNDRangeKernel(commandQueue, round_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	clFinish(commandQueue);
	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "call kernel round error : %s\n",
		    getCLErrorStr(clResult));
	    return -1;
	}

	clEnqueueFillBuffer(commandQueue, indexesE2, &initV, sizeof (int), 0,
			    tmpSize, 0, NULL, NULL);
	clFinish(commandQueue);
	constRound = 1;
	edges_a = EDGES_B;
	edges_b = EDGES_B / 2;
	clResult |= clSetKernelArg(round_kernel, 0, sizeof (int), &constRound);
	clResult |=
	    clSetKernelArg(round_kernel, 1, sizeof (cl_mem), (void *) &dipkeys);
	clResult |=
	    clSetKernelArg(round_kernel, 2, sizeof (cl_mem), (void *) &bufferB);
	clResult |=
	    clSetKernelArg(round_kernel, 3, sizeof (cl_mem), (void *) &bufferA);
	clResult |=
	    clSetKernelArg(round_kernel, 4, sizeof (cl_mem),
			   (void *) &indexesE);
	clResult |=
	    clSetKernelArg(round_kernel, 5, sizeof (cl_mem),
			   (void *) &indexesE2);
	clResult |= clSetKernelArg(round_kernel, 6, sizeof (int), &edges_a);
	clResult |= clSetKernelArg(round_kernel, 7, sizeof (int), &edges_b);
	clResult |=
	    clEnqueueNDRangeKernel(commandQueue, round_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	clFinish(commandQueue);

	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "call kernel round error : %s\n",
		    getCLErrorStr(clResult));
	    return -1;
	}

	clEnqueueFillBuffer(commandQueue, indexesE, &initV, sizeof (int), 0,
			    tmpSize, 0, NULL, NULL);
	clFinish(commandQueue);

	constRound = 2;
	edges_a = EDGES_B / 2;
	edges_b = EDGES_A / 4;
	clResult |= clSetKernelArg(round_kernel, 0, sizeof (int), &constRound);
	clResult |=
	    clSetKernelArg(round_kernel, 1, sizeof (cl_mem), (void *) &dipkeys);
	clResult |=
	    clSetKernelArg(round_kernel, 2, sizeof (cl_mem), (void *) &bufferA);
	clResult |=
	    clSetKernelArg(round_kernel, 3, sizeof (cl_mem), (void *) &bufferB);
	clResult |=
	    clSetKernelArg(round_kernel, 4, sizeof (cl_mem),
			   (void *) &indexesE2);
	clResult |=
	    clSetKernelArg(round_kernel, 5, sizeof (cl_mem),
			   (void *) &indexesE);
	clResult |= clSetKernelArg(round_kernel, 6, sizeof (int), &edges_a);
	clResult |= clSetKernelArg(round_kernel, 7, sizeof (int), &edges_b);
	clResult |=
	    clEnqueueNDRangeKernel(commandQueue, round_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	clFinish(commandQueue);

	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "call kernel round error : %s\n",
		    getCLErrorStr(clResult));
	    return -1;
	}

	clEnqueueFillBuffer(commandQueue, indexesE2, &initV, sizeof (int), 0,
			    tmpSize, 0, NULL, NULL);
	clFinish(commandQueue);

	constRound = 3;
	edges_a = EDGES_A / 4;
	edges_b = EDGES_B / 4;
	clResult |= clSetKernelArg(round_kernel, 0, sizeof (int), &constRound);
	clResult |=
	    clSetKernelArg(round_kernel, 1, sizeof (cl_mem), (void *) &dipkeys);
	clResult |=
	    clSetKernelArg(round_kernel, 2, sizeof (cl_mem), (void *) &bufferB);
	clResult |=
	    clSetKernelArg(round_kernel, 3, sizeof (cl_mem), (void *) &bufferA);
	clResult |=
	    clSetKernelArg(round_kernel, 4, sizeof (cl_mem),
			   (void *) &indexesE);
	clResult |=
	    clSetKernelArg(round_kernel, 5, sizeof (cl_mem),
			   (void *) &indexesE2);
	clResult |= clSetKernelArg(round_kernel, 6, sizeof (int), &edges_a);
	clResult |= clSetKernelArg(round_kernel, 7, sizeof (int), &edges_b);
	clResult |=
	    clEnqueueNDRangeKernel(commandQueue, round_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	clFinish(commandQueue);

	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "call kernel round error : %s\n",
		    getCLErrorStr(clResult));
	    return -1;
	}

	for (int round = 4; round < tp.ntrims; round += 2)
	{
	    clEnqueueFillBuffer(commandQueue, indexesE, &initV, sizeof (int), 0,
				tmpSize, 0, NULL, NULL);
	    clFinish(commandQueue);

	    constRound = round;
	    edges_a = EDGES_B / 4;
	    edges_b = EDGES_B / 4;
	    clSetKernelArg(round_kernel, 0, sizeof (int), &constRound);
	    clSetKernelArg(round_kernel, 1, sizeof (cl_mem), (void *) &dipkeys);
	    clSetKernelArg(round_kernel, 2, sizeof (cl_mem), (void *) &bufferA);
	    clSetKernelArg(round_kernel, 3, sizeof (cl_mem), (void *) &bufferB);
	    clSetKernelArg(round_kernel, 4, sizeof (cl_mem),
			   (void *) &indexesE2);
	    clSetKernelArg(round_kernel, 5, sizeof (cl_mem),
			   (void *) &indexesE);
	    clSetKernelArg(round_kernel, 6, sizeof (int), &edges_a);
	    clSetKernelArg(round_kernel, 7, sizeof (int), &edges_b);
	    clEnqueueNDRangeKernel(commandQueue, round_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	    clFinish(commandQueue);

	    clEnqueueFillBuffer(commandQueue, indexesE2, &initV, sizeof (int),
				0, tmpSize, 0, NULL, NULL);
	    clFinish(commandQueue);

	    constRound = round + 1;
	    edges_a = EDGES_B / 4;
	    edges_b = EDGES_B / 4;
	    clSetKernelArg(round_kernel, 0, sizeof (int), &constRound);
	    clSetKernelArg(round_kernel, 1, sizeof (cl_mem), (void *) &dipkeys);
	    clSetKernelArg(round_kernel, 2, sizeof (cl_mem), (void *) &bufferB);
	    clSetKernelArg(round_kernel, 3, sizeof (cl_mem), (void *) &bufferA);
	    clSetKernelArg(round_kernel, 4, sizeof (cl_mem),
			   (void *) &indexesE);
	    clSetKernelArg(round_kernel, 5, sizeof (cl_mem),
			   (void *) &indexesE2);
	    clSetKernelArg(round_kernel, 6, sizeof (int), &edges_a);
	    clSetKernelArg(round_kernel, 7, sizeof (int), &edges_b);
	    clEnqueueNDRangeKernel(commandQueue, round_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	    clWaitForEvents(1, &event);

	}

	clFinish(commandQueue);
	clEnqueueFillBuffer(commandQueue, indexesE, &initV, sizeof (int), 0,
			    tmpSize, 0, NULL, NULL);
	clFinish(commandQueue);
	cl_kernel tail_kernel = clCreateKernel(program, "Tail", NULL);
	global_work_size[0] = tp.tail.blocks * tp.tail.tpb;
	local_work_size[0] = tp.tail.tpb;
	int tail_edges = DUCK_B_EDGES / 4;
	clResult |=
	    clSetKernelArg(tail_kernel, 0, sizeof (cl_mem), (void *) &bufferA);
	clResult |=
	    clSetKernelArg(tail_kernel, 1, sizeof (cl_mem), (void *) &bufferB);
	clResult |=
	    clSetKernelArg(tail_kernel, 2, sizeof (cl_mem),
			   (void *) &indexesE2);
	clResult |=
	    clSetKernelArg(tail_kernel, 3, sizeof (cl_mem), (void *) &indexesE);
	clResult |= clSetKernelArg(tail_kernel, 4, sizeof (int), &tail_edges);
	clResult |=
	    clEnqueueNDRangeKernel(commandQueue, tail_kernel, 1, NULL,
				   global_work_size, local_work_size, 0, NULL,
				   &event);
	clFinish(commandQueue);
	clEnqueueReadBuffer(commandQueue, indexesE, CL_TRUE, 0,
			    NX * NY * sizeof (u32), hostA, 0, NULL, NULL);

	clFinish(commandQueue);
	if (clResult != CL_SUCCESS)
	{
	    fprintf(stderr, "call kernel round error : %s\n",
		    getCLErrorStr(clResult));
	    return -1;
	}

	return hostA[0];
    }

};