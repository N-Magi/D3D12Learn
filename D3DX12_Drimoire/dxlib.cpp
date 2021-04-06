#include "dxlib.h"


void dxLib::Init(HWND hWnd)
{
	//ComPtr<ID3D12Debug> debugLayer;
	//D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	//debugLayer->EnableDebugLayer();
	_hWnd = hWnd;
	HRESULT hr;
	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_device))))
		throw new std::runtime_error("Create Device Failure");

	CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	hr = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAlloc));
	hr = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC descCmdQueue = {};
	descCmdQueue.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	descCmdQueue.NodeMask = 0;
	descCmdQueue.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	descCmdQueue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hr = _device->CreateCommandQueue(&descCmdQueue, IID_PPV_ARGS(&_cmdQueue));

	
	GetClientRect(hWnd, &_windowSize);
	ComPtr<IDXGISwapChain1> sc1;
	DXGI_SWAP_CHAIN_DESC1 descSwapChain = {};
	descSwapChain.Width = _windowSize.right - _windowSize.left;
	descSwapChain.Height = _windowSize.bottom - _windowSize.top;
	descSwapChain.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descSwapChain.Stereo = false;
	descSwapChain.SampleDesc.Count = 1;
	descSwapChain.SampleDesc.Quality = 0;
	descSwapChain.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	descSwapChain.BufferCount = bufferCOunt;
	descSwapChain.Scaling = DXGI_SCALING_STRETCH;
	descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	descSwapChain.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	descSwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	hr = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(), _hWnd, &descSwapChain, nullptr, nullptr, &sc1);
	hr = sc1.As(&_swapChain);

	D3D12_DESCRIPTOR_HEAP_DESC descHeap_rtv = {};
	descHeap_rtv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeap_rtv.NodeMask = 0;
	descHeap_rtv.NumDescriptors = bufferCOunt;
	descHeap_rtv.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = _device->CreateDescriptorHeap(&descHeap_rtv, IID_PPV_ARGS(&_heapRtv));

	/*
	D3D12_DESCRIPTOR_HEAP_DESC descHeap_texture = {};
	descHeap_texture.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeap_texture.NodeMask = 0;
	descHeap_texture.NumDescriptors = 1;
	descHeap_texture.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;;
	hr = _device->CreateDescriptorHeap(&descHeap_texture, IID_PPV_ARGS(&_heapTexture));
	
	//‚±‚±‚Å‚¢‚¢‚Ì‚©“ä
	D3D12_SHADER_RESOURCE_VIEW_DESC descSrv = {};
	descSrv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descSrv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	descSrv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	descSrv.Texture2D.MipLevels = 1;
	_device->CreateShaderResourceView(_buff_Texture.Get(), &descSrv, _heapTexture->GetCPUDescriptorHandleForHeapStart());
	*/

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	hr = _swapChain->GetDesc(&swcDesc);
	_buffer_Rtv = std::vector<ComPtr<ID3D12Resource>>(swcDesc.BufferCount);
	//std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = _heapRtv->GetCPUDescriptorHandleForHeapStart();
	D3D12_RENDER_TARGET_VIEW_DESC descRtv = {};
	descRtv.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	descRtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (size_t i = 0; i < swcDesc.BufferCount; ++i) {
		hr = _swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_buffer_Rtv[i]));
		_device->CreateRenderTargetView(_buffer_Rtv[i].Get(), &descRtv, handle);
		handle.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	UINT64 _fenceVal = 0;
	hr = _device->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fences));

	Prepare();
}

void dxLib::Render()
{
	//_cmdAlloc->Reset();
	auto bbIdx = _swapChain->GetCurrentBackBufferIndex();
	auto handle_Rtv = _heapRtv->GetCPUDescriptorHandleForHeapStart();
	handle_Rtv.ptr += bbIdx * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_RESOURCE_BARRIER descBarrier = {};
	descBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	descBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	descBarrier.Transition.pResource = _buffer_Rtv[bbIdx].Get();
	descBarrier.Transition.Subresource = 0;
	descBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	descBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &descBarrier);
	
	

	_cmdList->OMSetRenderTargets(1, &handle_Rtv, true, nullptr);

	float clearColor[] = { 0.0f,0.0f,0.0f,1.0f };
	_cmdList->ClearRenderTargetView(handle_Rtv, clearColor, 0, nullptr);

	GenCommand();

	descBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	descBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &descBarrier);

	
	_cmdList->Close();

	ID3D12CommandList* cmdList[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdList);

	WaitForGpu();

	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	_swapChain->Present(1, 0);

	
}

void dxLib::WaitForGpu()
{
	_cmdQueue->Signal(_fences.Get(), ++_fenceVal);
	if (_fences->GetCompletedValue() != _fenceVal)
	{
		auto gevent = CreateEvent(nullptr, false, false, nullptr);
		_fences->SetEventOnCompletion(_fenceVal, gevent);
		WaitForSingleObject(gevent, INFINITE);
		CloseHandle(gevent);
	}

}