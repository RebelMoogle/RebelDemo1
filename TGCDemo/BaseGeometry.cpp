#include "DXUT.h"
#include "BaseGeometry.h"

BaseGeometry::BaseGeometry(bool isItStatic) :
mIsStatic(isItStatic),
mVertexBuffer(NULL),
mIndexBuffer(NULL),
mNumElements(0),
mStride(0),
mIndexFormat(DXGI_FORMAT_R16_UINT),
mTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
{

}


BaseGeometry::~BaseGeometry(void)
{
	SAFE_RELEASE(mVertexBuffer);
	SAFE_RELEASE(mIndexBuffer);
}

void BaseGeometry::Bind( ID3D11DeviceContext* ImmediateContext )
{
	ImmediateContext->IASetIndexBuffer(mIndexBuffer, mIndexFormat, 0);
	UINT offset = 0;
	ImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &mStride, &offset);
	ImmediateContext->IASetPrimitiveTopology(mTopology);
}

void BaseGeometry::Render( ID3D11DeviceContext* ImmediateContext )
{
	if (mIndexBuffer)
		ImmediateContext->DrawIndexed(mNumElements, 0, 0);
	else ImmediateContext->Draw(mNumElements, 0);
}

bool BaseGeometry::IsStatic() const
{
	return mIsStatic;
}



