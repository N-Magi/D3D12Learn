#include "vertex.h"

using namespace DirectX;

void vertexApp::Prepare()
{
	
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
	float k = 0.7f;
	Vertex vertices[] = {
		{{-k, -k, 0.0f},{0.0f,1.0f}},
		{{-k, k, 0.0f},{0.0f,0.0f}},
		{{k, -k, 0.0f},{1.0f,1.0f}},
		{{k, k, 0.0f},{1.0f,0.0f}}
	};

	unsigned short indices[] = { 0,1,2,1,2,3 };

	//Vertex
	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC descRes = {};
	descRes.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	descRes.Width = sizeof(vertices);
	descRes.Height = 1;
	descRes.DepthOrArraySize = 1;
	descRes.MipLevels = 1;
	descRes.Format = DXGI_FORMAT_UNKNOWN;
	descRes.SampleDesc.Count = 1; //アンチエイリアス
	descRes.Flags = D3D12_RESOURCE_FLAG_NONE;
	descRes.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	_device->CreateCommittedResource(
		&heapprop, D3D12_HEAP_FLAG_NONE,
		&descRes, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&_buff_Vertex));

	//UV
	descRes.Width = sizeof(indices);

	_device->CreateCommittedResource(
		&heapprop, D3D12_HEAP_FLAG_NONE,
		&descRes, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&_buff_Index));
	
	

	

	Vertex* vertMap = nullptr;
	HRESULT hr = _buff_Vertex->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	_buff_Vertex->Unmap(0, nullptr);

	unsigned short* idxMap = nullptr;
	hr = _buff_Index->Map(0, nullptr, (void**)&idxMap);
	std::copy(std::begin(indices), std::end(indices), idxMap);
	_buff_Index->Unmap(0, nullptr);

	
	_buffView_vertex.BufferLocation = _buff_Vertex->GetGPUVirtualAddress();
	_buffView_vertex.SizeInBytes = sizeof(vertices); //全バイト数
	_buffView_vertex.StrideInBytes = sizeof(vertices[0]);//単頂点あたりのバイト数
	_cmdList->IASetVertexBuffers(0, 1, &_buffView_vertex);

	_buffView_Index.BufferLocation = _buff_Index->GetGPUVirtualAddress();
	_buffView_Index.SizeInBytes = sizeof(indices);
	_buffView_Index.Format = DXGI_FORMAT_R16_UINT;
	_cmdList->IASetIndexBuffer(&_buffView_Index);

	

	ComPtr<ID3DBlob> err_Blob;
	hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_vsBlob, &err_Blob);
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_psBlob, &err_Blob);

	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	D3D12_ROOT_PARAMETER paramRoot = {};
	paramRoot.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	paramRoot.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_DESCRIPTOR_RANGE rangeDescTbl = {};
	rangeDescTbl.NumDescriptors = 1;
	rangeDescTbl.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeDescTbl.BaseShaderRegister = 0;
	rangeDescTbl.OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	paramRoot.DescriptorTable.pDescriptorRanges = &rangeDescTbl;
	paramRoot.DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC descSampler = {};
	descSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	descSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	descSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	descSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	descSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	descSampler.MaxLOD = D3D12_FLOAT32_MAX;
	descSampler.MinLOD = 0.0f;
	descSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	D3D12_ROOT_SIGNATURE_DESC descRootSig = {};
	descRootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descRootSig.pParameters = &paramRoot;
	descRootSig.NumParameters = 1;
	descRootSig.pStaticSamplers = &descSampler;
	descRootSig.NumStaticSamplers = 1;

	ComPtr<ID3DBlob> errBlob_root;
	ID3DBlob* rootBlob = nullptr;
	hr = D3D12SerializeRootSignature(
		&descRootSig,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootBlob, &errBlob_root);
	hr = _device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
	rootBlob->Release();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_gpipeline = {};
	desc_gpipeline.pRootSignature = _rootSignature.Get();
	desc_gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	desc_gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	desc_gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	desc_gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();
	desc_gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	desc_gpipeline.RasterizerState.MultisampleEnable = false;
	desc_gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc_gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc_gpipeline.RasterizerState.DepthClipEnable = true;

	desc_gpipeline.BlendState.AlphaToCoverageEnable = false;
	desc_gpipeline.BlendState.IndependentBlendEnable = false;
	D3D12_RENDER_TARGET_BLEND_DESC descBlend = {};
	descBlend.BlendEnable = false;
	descBlend.LogicOpEnable = false;
	descBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	desc_gpipeline.BlendState.RenderTarget[0] = descBlend;

	desc_gpipeline.InputLayout.pInputElementDescs = inputElementDesc;
	desc_gpipeline.InputLayout.NumElements = _countof(inputElementDesc);

	desc_gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	desc_gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	desc_gpipeline.NumRenderTargets = 1;
	desc_gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc_gpipeline.SampleDesc.Count = 1;
	desc_gpipeline.SampleDesc.Quality = 0;

	hr = _device->CreateGraphicsPipelineState(&desc_gpipeline, IID_PPV_ARGS(&_pipelineState));

	_viewport.Width = _windowSize.right - _windowSize.left;
	_viewport.Height = _windowSize.bottom - _windowSize.top;
	_viewport.TopLeftX = 0;
	_viewport.TopLeftY = 0;
	_viewport.MaxDepth = 1.0f;
	_viewport.MinDepth = 0.0f;
	_scissorRect.top = 0;
	_scissorRect.left = 0;
	_scissorRect.right = _scissorRect.left + _windowSize.right - _windowSize.left;
	_scissorRect.bottom = _scissorRect.top + _windowSize.bottom - _windowSize.top;

	/*struct TexRGBA
	{
		unsigned char R, G, B, A;
	};
	std::vector<TexRGBA> textureData(256 * 256);
	for (auto& rgba : textureData)
	{
		rgba.A = 255;
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
	}*/

	TexMetadata metaData = {};
	ScratchImage scratchImg = {};

	hr = LoadFromWICFile(L"img/textest200x200.png", WIC_FLAGS_NONE, &metaData, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);


	//Texture
	heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	heapprop.CreationNodeMask = 0;
	heapprop.VisibleNodeMask = 0;

	descRes.Format = metaData.format;//DXGI_FORMAT_R8G8B8A8_UNORM;
	descRes.Width = metaData.width;//256;
	descRes.Height = metaData.height;//256;
	descRes.DepthOrArraySize = metaData.depth;//1;
	descRes.SampleDesc.Count = 1;
	descRes.SampleDesc.Quality = 0;
	descRes.MipLevels = metaData.mipLevels;//1;
	descRes.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData.dimension);//D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	descRes.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	descRes.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES propUploadHeap = {};
	propUploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

	propUploadHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	propUploadHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	
	propUploadHeap.CreationNodeMask = 0;
	propUploadHeap.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC descUpRes = {};
	descUpRes.Format = DXGI_FORMAT_UNKNOWN;
	descUpRes.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	descUpRes.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height;
	descUpRes.Height = 1;
	descUpRes.DepthOrArraySize = 1;
	descUpRes.MipLevels = 1;
	descUpRes.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	descUpRes.Flags = D3D12_RESOURCE_FLAG_NONE;
	descUpRes.SampleDesc.Count = 1;
	descUpRes.SampleDesc.Quality = 0;

	ComPtr<ID3D12Resource> upBuff = nullptr;

	_device->CreateCommittedResource(&propUploadHeap, D3D12_HEAP_FLAG_NONE, &descUpRes, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&_buff_UpTexture));

	_device->CreateCommittedResource(
		&heapprop, D3D12_HEAP_FLAG_NONE,
		&descRes, D3D12_RESOURCE_STATE_COPY_DEST,//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr, IID_PPV_ARGS(&_buff_Texture));

	uint8_t* map4img = nullptr;
	hr = _buff_UpTexture->Map(0, nullptr, (void**)&map4img);
	auto srcAddr = img->pixels;
	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < img->height; y++)
	{
		std::copy_n(srcAddr, img->rowPitch, map4img);
		srcAddr += img->rowPitch;
		map4img += rowPitch;
	}
	
	_buff_UpTexture->Unmap(0, nullptr);

	//D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = _buff_UpTexture;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = metaData.width;
	src.PlacedFootprint.Footprint.Height = metaData.height;
	src.PlacedFootprint.Footprint.Depth = metaData.depth;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);//img->rowPitch;
	src.PlacedFootprint.Footprint.Format = img->format;
	
	//D3D12_TEXTURE_COPY_LOCATION dest = {};
	dest.pResource = _buff_Texture;
	dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dest.SubresourceIndex = 0;

	//_buff_Texture->WriteToSubresource(0, nullptr, textureData.data(), sizeof(TexRGBA) * 256, sizeof(TexRGBA)* textureData.size());
	//_buff_Texture->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);

	D3D12_DESCRIPTOR_HEAP_DESC descHeap_texture = {};
	descHeap_texture.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeap_texture.NodeMask = 0;
	descHeap_texture.NumDescriptors = 1;
	descHeap_texture.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;;
	hr = _device->CreateDescriptorHeap(&descHeap_texture, IID_PPV_ARGS(&_heapTexture));

	D3D12_SHADER_RESOURCE_VIEW_DESC descSrv = {};
	descSrv.Format = metaData.format;//DXGI_FORMAT_R8G8B8A8_UNORM;
	descSrv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	descSrv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	descSrv.Texture2D.MipLevels = 1;
	_device->CreateShaderResourceView(_buff_Texture, &descSrv, _heapTexture->GetCPUDescriptorHandleForHeapStart());

	D3D12_RESOURCE_BARRIER descTexBarrier = {};
	descTexBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	descTexBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	descTexBarrier.Transition.pResource = _buff_Texture;
	descTexBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	descTexBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	descTexBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	

	_cmdList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
	_cmdList->ResourceBarrier(1, &descTexBarrier);
	_cmdList->Close();

	ID3D12CommandList* cmdList[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdList);
	WaitForGpu();

	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);
	
}

void vertexApp::GenCommand()
{
	_cmdList->SetPipelineState(_pipelineState.Get());

	_cmdList->RSSetViewports(1, &_viewport);
	_cmdList->RSSetScissorRects(1, &_scissorRect);

	_cmdList->SetGraphicsRootSignature(_rootSignature.Get());
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_cmdList->IASetVertexBuffers(0, 1, &_buffView_vertex);
	_cmdList->IASetIndexBuffer(&_buffView_Index);

	_cmdList->SetGraphicsRootSignature(_rootSignature.Get());
	_cmdList->SetDescriptorHeaps(1, &_heapTexture);
	_cmdList->SetGraphicsRootDescriptorTable(0, _heapTexture->GetGPUDescriptorHandleForHeapStart());

	_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	
}
