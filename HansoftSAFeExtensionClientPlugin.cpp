/*

TODO:
* Only display the menu in the product backlog on Leaf Items (Ideally in the Portfolio ) Should add some checks also that there is a Feature backlog and that the target project is a program.
* Improving the dialogue layout
*/



#include "../../Projects/HansoftSDK_7_105/HPMSdkCpp.h"

#include <tchar.h>
#include <conio.h>
#include <windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <memory>

#define mod_export __declspec(dllexport)

using namespace std;
using namespace HPMSdk;

class CHansoftSAFeExtension_ClientPlugin : public HPMSdkCallbacks
{
public:

	struct CDynamicHelper
	{
		HPMNotificationSubscription m_RightClickSubscription;
		HPMNotificationSubscription m_DynamicUpdateSubscription;
	};

	std::vector<HPMUniqueID> m_LastSelectedTasks;

	HPMString m_selectedFeatureName;
	HPMString m_selectedProgram;

	CHansoftSAFeExtension_ClientPlugin(const void *_pClientData)
	{
		m_pSession = nullptr;

		try
		{
			m_pSession = HPMSdkSession::SessionOpen(hpm_str(""), 0, hpm_str(""), hpm_str(""), hpm_str(""), this, NULL, true, EHPMSdkDebugMode_Off, _pClientData, 0, hpm_str(""), HPMSystemString(), NULL);
			m_IntegrationIdentifier = hpm_str("com.hansoft.safeextension.clientplugin");
			m_UserContext = (void *)23183;
		}
		catch (HPMSdkException &_Exception)
		{
			HPMString ErrorStr = _Exception.GetAsString();

			wstringstream Stream;
			Stream << hpm_str("Error initializing SAFe Extension Client Plugin:\r\n\r\n");
			Stream << ErrorStr;
			MessageBox(NULL, Stream.str().c_str(), hpm_str("SAFe Extension Client Plugin Error"), MB_OK|MB_ICONERROR);
		}
		catch (HPMSdkCppException &_Exception)
		{
			wstringstream Stream; 
			Stream << hpm_str("Error initializing SAFe Extension Client Plugin:\r\n\r\n");
			Stream << _Exception.what();
			MessageBox(NULL, Stream.str().c_str(), hpm_str("SAFe Extension Client Plugin Error"), MB_OK|MB_ICONERROR);
		}
	}

	~CHansoftSAFeExtension_ClientPlugin()
	{ 
		m_pDynamicHelper.reset();
		if (m_pSession)
		{
			HPMSdkSession::SessionDestroy(m_pSession);
			m_pSession = nullptr;
		}
	}

	virtual void On_ProcessError(EHPMError _Error)
	{
	}

	virtual void On_Callback(const HPMChangeCallbackData_ClientSyncDone &_Data)
	{

		try
		{

			m_CustomDialogSpec = 
				hpm_str("	DialogName \"Add Feature\"\r\n")
				hpm_str("	Item Tab\r\n")
				hpm_str("	{\r\n")
				hpm_str("		Identifier \"com.hansoft.safeextension.clientplugin.addfeaturedialog\"\r\n")
				hpm_str("		Name \"Info\"\r\n")
				hpm_str("		InfoText \"Custom dynamic SDK dialog\"\r\n")	
				hpm_str("		LayoutStyle \"VerticalList\"\r\n")
				hpm_str("		Item FormLayout\r\n")
				hpm_str("		{\r\n")
				hpm_str("			Identifier \"Form\"\r\n")
				hpm_str("			Item ComboBoxes\r\n")
				hpm_str("			{\r\n")
				hpm_str("				Name \"Select a program and give a name for the feature\"\r\n")
				hpm_str("				Identifier \"ComboBoxes\"\r\n")
				hpm_str("				Columns 1\r\n")
				hpm_str("				ColumnsOrder 1\r\n")
				hpm_str("				Item\r\n")
				hpm_str("				{\r\n")
				hpm_str("					Identifier \"ProgramCombo\"\r\n")
				hpm_str("					Name \"Program:\"\r\n")
				hpm_str("					DefaultValue \"Program:\"\r\n")
				hpm_str("					Choices \"Projects\"\r\n")
				hpm_str("					Sorted 1\r\n")
				hpm_str("				}\r\n")
				hpm_str("			}\r\n")
				hpm_str("			Item Edit\r\n")
				hpm_str("			{\r\n")
				hpm_str("				Identifier \"FeatureName\"\r\n")
				hpm_str("				Name \"Feature name:\"\r\n")
				hpm_str("				DefaultValue \"\"\r\n")
				hpm_str("				Password 0\r\n")
				hpm_str("			}\r\n")
				hpm_str("		}\r\n")
				hpm_str("		Item FormLayout\r\n")
				hpm_str("		{\r\n")
				hpm_str("			Identifier \"Form2\"\r\n")
				hpm_str("		Item StaticText\r\n")
				hpm_str("		{\r\n")
				hpm_str("			Name \"Information\"\r\n")
				hpm_str("			Identifier \"EpicInfo\"\r\n")
				hpm_str("			DefaultValue \"The new feature will be added to the Feature backlog section of the selected Program and linked to the selected epic.\"\r\n")
				hpm_str("		}\r\n")
				hpm_str("		}\r\n")
				hpm_str("	}\r\n");

			m_pDynamicHelper = shared_ptr<CDynamicHelper>(new CDynamicHelper());
			m_pDynamicHelper->m_RightClickSubscription = m_pSession->GlobalRegisterForRightClickNotifications(NULL);
			m_pDynamicHelper->m_DynamicUpdateSubscription = m_pSession->GlobalRegisterForDynamicCustomSettingsNotifications(hpm_str("com.hansoft.safeextension.clientplugin."), m_UserContext);
		}
		catch (const HPMSdk::HPMSdkException &_Exception)
		{
			if (_Exception.GetError() == EHPMError_ConnectionLost)
				return;
		}
		catch (const HPMSdk::HPMSdkCppException &)
		{
		}
	}

	virtual void On_Callback(const HPMChangeCallbackData_RightClickDisplayTaskMenu &_Data)
	{
		try
		{
			if (_Data.m_SelectedTasks.size() == 1 &&
				m_pSession->UtilIsIDBacklogProject(m_pSession->TaskGetContainer(m_pSession->TaskRefGetTask(_Data.m_SelectedTasks[0]))) &&
				m_pSession->ProjectGetProperties(m_pSession->UtilGetRealProjectIDFromProjectID(_Data.m_ProjectID)).m_Name.find(hpm_str("Portfolio")) != -1
				)
			{
				m_LastSelectedTasks = _Data.m_SelectedTasks;

				m_pSession->GlobalAddRightClickMenuItem
					(
					_Data.m_RightClickContext
					, hpm_str(""), m_IntegrationIdentifier + hpm_str(".taskmenu.safeclientplugin.root")
					, m_pSession->LocalizationCreateUntranslatedStringFromString(hpm_str("SAFe"))
					, NULL
					)
					;
				if (m_LastSelectedTasks.size() == 1)
				{
					m_pSession->GlobalAddRightClickMenuItem
						(
						_Data.m_RightClickContext
						, m_IntegrationIdentifier + hpm_str(".taskmenu.safeclientplugin.root")
						, m_IntegrationIdentifier + hpm_str(".taskmenu.safeclientplugin.addfeature")
						, m_pSession->LocalizationCreateUntranslatedStringFromString(hpm_str("Add feature..."))
						, NULL
						)
						;
				}
				m_pSession->GlobalAddRightClickMenuItem
					(
					_Data.m_RightClickContext
					, m_IntegrationIdentifier + hpm_str(".taskmenu.safeclientplugin.root")
					, m_IntegrationIdentifier + hpm_str(".taskmenu.safeclientplugin.about")
					, m_pSession->LocalizationCreateUntranslatedStringFromString(hpm_str("About..."))
					, NULL
					)
					;
			}
		}
		catch (const HPMSdk::HPMSdkException &_Exception)
		{
			if (_Exception.GetError() == EHPMError_ConnectionLost)
				return;
		}
		catch (const HPMSdk::HPMSdkCppException &)
		{
		}
	}

	virtual void On_Callback(const HPMChangeCallbackData_RightClickMenuItemSelected &_Data)
	{
		try
		{
			if (_Data.m_UniqueName == m_IntegrationIdentifier + hpm_str(".taskmenu.safeclientplugin.about"))
			{
				MessageBox(NULL, hpm_str("The Hansoft Scaled Agile Framework Extension Client Plugin adds functionality to support working with the Scaled Agile Framework in Hansoft. See http://github.com/hansoft/SAFeKit for more information."), hpm_str("Scaled Agile Framework Extension Client Plugin"), MB_OK|MB_ICONINFORMATION);
			}
			else if  (_Data.m_UniqueName == m_IntegrationIdentifier + hpm_str(".taskmenu.safeclientplugin.addfeature"))
			{
				m_selectedFeatureName = hpm_str("");
				m_selectedProgram = hpm_str("");
				HPMString SelectedEpicName;
				SelectedEpicName = m_pSession->TaskGetDescription(m_pSession->TaskRefGetTask(m_LastSelectedTasks[0]));

				HPMString InitialValues =
					hpm_str("com.hansoft.safeextension.clientplugin.addfeaturedialog\r\n")
					hpm_str("{\r\n")
					hpm_str("}");
				m_pSession->GlobalDisplayCustomSettingsDialog
					(
					HPMUniqueID()							// Can be set to specific project
					, HPMUniqueID()							// Can be set to specific resource
					, m_CustomDialogSpec
					, InitialValues
					)
					;
			}
		}
		catch (const HPMSdk::HPMSdkException &_Exception)
		{
			if (_Exception.GetError() == EHPMError_ConnectionLost)
				return;
		}
		catch (const HPMSdk::HPMSdkCppException &)
		{
		}
	}

	virtual void On_Callback(const HPMChangeCallbackData_DynamicCustomSettingsValueChanged &_Data)
	{
		try
		{
			if (_Data.m_UserContext != m_UserContext)
				return;

			if (_Data.m_Path == hpm_str("com.hansoft.safeextension.clientplugin.addfeaturedialog/Form/FeatureName"))
				m_selectedFeatureName = _Data.m_Value;
			else if (_Data.m_Path == hpm_str("com.hansoft.safeextension.clientplugin.addfeaturedialog/Form/ComboBoxes/ProgramCombo"))
				m_selectedProgram = _Data.m_Value;
		}
		catch (const HPMSdk::HPMSdkException &_Exception)
		{
			if (_Exception.GetError() == EHPMError_ConnectionLost)
				return;
		}
		catch (const HPMSdk::HPMSdkCppException &)
		{
		}
	}

	virtual void On_Callback(const HPMChangeCallbackData_DynamicCustomSettingsNotification &_Data)
	{
		if (! (_Data.m_TabIdentifiers.size() == 1 && _Data.m_TabIdentifiers[0].compare(hpm_str("com.hansoft.safeextension.clientplugin.addfeaturedialog")) == 0))
			return;


		if (_Data.m_Notification == EHPMDynamicCustomSettingsNotification_DialogEndedOk)
		{
			if ((m_selectedProgram.compare(hpm_str("")) != 0) && (m_selectedFeatureName.compare(hpm_str("")) != 0))
			{
				int proj_id = std::stoi(m_selectedProgram.substr(6));

				HPMProjectEnum Projects = m_pSession->ProjectEnum();
				for (HPMUInt32 i = 0; i < Projects.m_Projects.size(); ++i)
				{
					HPMUniqueID ProjectUID = Projects.m_Projects[i];
					if (ProjectUID.m_ID == proj_id)
					{
						HPMProjectProperties ProjectProp = m_pSession->ProjectGetProperties(ProjectUID);
						HPMUniqueID ProductBacklogUID = m_pSession->ProjectUtilGetBacklog(ProjectUID);
						HPMTaskEnum Tasks = m_pSession->TaskEnum(ProductBacklogUID);
						for (HPMUInt32 j=0; j < Tasks.m_Tasks.size(); ++j)
						{
							HPMUniqueID FeatureBacklogUID = Tasks.m_Tasks[j];
							if (m_pSession->TaskGetDescription(FeatureBacklogUID).compare(hpm_str("Feature backlog")) == 0)
							{
								HPMTaskCreateUnified NewTask;
								NewTask.m_Tasks.resize(1);
								NewTask.m_Tasks[0].m_bIsProxy = false;
								NewTask.m_Tasks[0].m_LocalID = -1;
								NewTask.m_Tasks[0].m_TaskLockedType = EHPMTaskLockedType_BacklogItem;

								HPMTaskCreateUnifiedReference PrevRefID;
								PrevRefID.m_bLocalID = false;
								PrevRefID.m_RefID = -1;
								NewTask.m_Tasks[0].m_PreviousRefID = PrevRefID;

								HPMTaskCreateUnifiedReference PrevWorkPrioRefID;
								PrevWorkPrioRefID.m_bLocalID = false;
								PrevWorkPrioRefID.m_RefID = -2;
								NewTask.m_Tasks[0].m_PreviousWorkPrioRefID = PrevWorkPrioRefID;

								HPMTaskCreateUnifiedReference FeatureBacklogTaskRefID;
								FeatureBacklogTaskRefID.m_bLocalID = false;
								FeatureBacklogTaskRefID.m_RefID = m_pSession->TaskGetMainReference(FeatureBacklogUID);
								NewTask.m_Tasks[0].m_ParentRefIDs.push_back(FeatureBacklogTaskRefID);

								NewTask.m_Tasks[0].m_NonProxy_ReuseID = 0;
								NewTask.m_Tasks[0].m_NonProxy_WorkflowID = 0xffffffff;

								HPMChangeCallbackData_TaskCreateUnified TaskCreateReturn = m_pSession->TaskCreateUnifiedBlock(ProductBacklogUID, NewTask);
								if (NewTask.m_Tasks.size() == 1)
								{
									// The returned is a task ref in the project container. We need the task id not the reference id.
									HPMUniqueID NewFeatureTaskRefID = TaskCreateReturn.m_Tasks[0].m_TaskRefID;
									HPMUniqueID NewFeatureTaskID = m_pSession->TaskRefGetTask(NewFeatureTaskRefID);

									// Set the name and the status of the feature
									m_pSession->TaskSetDescription(NewFeatureTaskID, m_selectedFeatureName);
									m_pSession->TaskSetStatus(NewFeatureTaskID, EHPMTaskStatus_NotDone, true, EHPMTaskSetStatusFlag_All);

									// Create the link from the created feature to the Epic
									HPMTaskLinkedTo LinkedTo;
									LinkedTo.m_LinkedTo.resize(1);
									HPMUniqueID SelectedEpicUID = m_pSession->TaskRefGetTask(m_LastSelectedTasks[0]);
									if (m_pSession->UtilIsIDTaskRef(SelectedEpicUID))
									{
										LinkedTo.m_LinkedTo[0].m_LinkedTo = SelectedEpicUID;
									}
									else
									{
										LinkedTo.m_LinkedTo[0].m_LinkedTo = m_pSession->TaskGetMainReference(SelectedEpicUID);
									}
									LinkedTo.m_LinkedTo[0].m_LinkedToType = EHPMTaskLinkedToLinkType_TaskItemOrBug;
									m_pSession->TaskSetLinkedTo(NewFeatureTaskID, LinkedTo);

									// Add a link from the Epic to the created feature
									HPMTaskLinkedTo LinkedFeatures = m_pSession->TaskGetLinkedTo(SelectedEpicUID);
									HPMUInt32 newSize = LinkedFeatures.m_LinkedTo.size()+1;
									LinkedFeatures.m_LinkedTo.resize(newSize);
									LinkedFeatures.m_LinkedTo[newSize-1].m_LinkedTo = NewFeatureTaskRefID;
									LinkedFeatures.m_LinkedTo[newSize-1].m_LinkedToType = EHPMTaskLinkedToLinkType_TaskItemOrBug;
									m_pSession->TaskSetLinkedTo(SelectedEpicUID, LinkedFeatures);

									m_pSession->TaskSetFullyCreated(NewFeatureTaskID);
									MessageBox(NULL,  hpm_str("The feature was created"), hpm_str("Hansoft SAFe plugin"), MB_OK|MB_ICONINFORMATION);
								}
								break;
							}
						}

					}
				}
			}
			else
			{
				MessageBox(NULL,  hpm_str("No feature was created. You have to select Program and give a name for the feature."), hpm_str("Hansoft SAFe plugin"), MB_OK|MB_ICONWARNING);
			}
		}
	}


	HPMUserContext m_UserContext;
	shared_ptr<CDynamicHelper> m_pDynamicHelper;
	HPMSdkSession *m_pSession;
	HPMString m_CustomDialogSpec;
	HPMString m_IntegrationIdentifier;

};

CHansoftSAFeExtension_ClientPlugin *g_pClientPlugin;

extern "C" mod_export void DHPMSdkCallingConvention HPMClientSDKCreate(const void *_pClientData)
{
	g_pClientPlugin = new CHansoftSAFeExtension_ClientPlugin(_pClientData);
}

extern "C" mod_export void DHPMSdkCallingConvention HPMClientSDKDestroy()
{
	delete g_pClientPlugin;
}
