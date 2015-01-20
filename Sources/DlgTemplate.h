#pragma once

//////////////////////////////////////////////////////////////////////////
// Класс для создания диалогов из DLGTEMPLATE
// Используется при загрузке диалога в зависимости от языка
//
class CResDlgTemplate
{
	public:

		CResDlgTemplate()
			: m_pData(NULL)
		{
		}

		~CResDlgTemplate()
		{
		}

	private:

		LPDLGTEMPLATE m_pData;

	public:

		VOID Create(LPDLGTEMPLATE pData)
		{
			m_pData = pData;
		}

		BOOL IsValid() const
		{
			return (NULL != m_pData);
		}

		LPDLGTEMPLATE GetTemplatePtr()
		{
			return m_pData;
		}
};
