#pragma once

#include <DirectXMath.h>
#include "dxlib.h"
#include <d3dcompiler.h>
#include <DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DirectXTex.lib")

class vertexApp :public dxLib
{
public:
	vertexApp() : dxLib() {}
	virtual void Prepare();
	virtual void GenCommand();
	//virtual size_t AlignmentedSize(size_t size, size_t alignment);
private:
	ComPtr<ID3D12Resource> _buff_Vertex;
	ComPtr<ID3D12Resource> _buff_Index;


	D3D12_VERTEX_BUFFER_VIEW _buffView_vertex;
	D3D12_INDEX_BUFFER_VIEW _buffView_Index;

	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ComPtr<ID3D12RootSignature> _rootSignature = nullptr;
	ComPtr<ID3D12PipelineState> _pipelineState = nullptr;

	D3D12_TEXTURE_COPY_LOCATION src, dest;

	D3D12_VIEWPORT _viewport = {};
	D3D12_RECT _scissorRect = {};
};
