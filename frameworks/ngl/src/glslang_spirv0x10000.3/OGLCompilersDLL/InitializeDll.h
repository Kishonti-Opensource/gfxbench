namespace glslang
{
	void InitializeMemoryPools();
	void FreeGlobalPools();

	inline bool InitThread()
	{
		InitializeMemoryPools();
		return true;
	}
	inline bool DetachThread()
	{
		FreeGlobalPools();
		return true;
	}
	inline void InitGlobalLock()
	{
	}
	inline bool InitProcess()
	{
		return true;
	}
	inline void GetGlobalLock()
	{
	}
	inline void ReleaseGlobalLock()
	{
	}
}
