#pragma once
#include <Windows.h>
#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG
#include <wrl/client.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <vector>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
//#include <Wsdclient.h>

using namespace std;
using namespace Microsoft::WRL;

class dxLib
{
public:
	void Init(HWND hWnd);
	
	void WaitForGpu();

	virtual void Render();
	virtual void Prepare() {}
	virtual void GenCommand() {}
	size_t AlignmentedSize(size_t size, size_t alignment) {return size + alignment - size % alignment; };

	int bufferCOunt = 2; //ダブルバッファ
protected:
	HWND _hWnd;
	RECT _windowSize;
	ComPtr<ID3D12Device> _device;
	ComPtr<IDXGIFactory6> _dxgiFactory;
	ComPtr<IDXGISwapChain4> _swapChain;
	ComPtr<ID3D12CommandQueue> _cmdQueue;
	ComPtr<ID3D12GraphicsCommandList> _cmdList;
	ComPtr<ID3D12CommandAllocator> _cmdAlloc;

	ComPtr<ID3D12DescriptorHeap> _heapRtv;
	std::vector<ComPtr<ID3D12Resource>> _buffer_Rtv;

	ID3D12DescriptorHeap* _heapTexture = nullptr;
	ID3D12Resource* _buff_Texture = nullptr;
	ID3D12Resource* _buff_UpTexture = nullptr;


	UINT64 _fenceVal = 0;
	//std::vector<ComPtr<ID3D12Fence>> _fences;
	ComPtr<ID3D12Fence> _fences;
};
