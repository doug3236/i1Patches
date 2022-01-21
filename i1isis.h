#pragma once
#include "tiffresults.h"
#include "cgats.h"

void make_i1isis(const vector<RGB>& patch_list_all, const string& imagename, int page_count);
void make_i1pro2(const vector<RGB>& patch_list_all, const string& imagename, int page_count);
void make_isis_txf(const vector<RGB>& patch_list_all, const string& imagename, int page_count);
void make_i1pro2_txf(const vector<RGB>& patch_list_all, const string& imagename, int page_count);

static const char* txf_0 = R"x(<?xml version="1.0" encoding="UTF-8"?>
<cc:CxF xmlns:cc="http://colorexchangeformat.com/CxF3-core" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
	<cc:FileInformation>
		<cc:Creator>X-Rite - Prism</cc:Creator>
		<cc:CreationDate>2021-12-24T16:34:56Z</cc:CreationDate>
		<cc:Description>Prism CXF3 file</cc:Description>
	</cc:FileInformation>
	<cc:Resources>
		<cc:ObjectCollection>)x";
static const char* txf_1 = R"x(
		<cc:Object ObjectType="Target" Name="Target1" Id="c%d">
				<cc:CreationDate>2021-12-24T16:34:56Z</cc:CreationDate>
				<cc:DeviceColorValues>
					<cc:ColorRGB ColorSpecification="Unknown">
						<cc:R>%d</cc:R>
						<cc:G>%d</cc:G>
						<cc:B>%d</cc:B>
					</cc:ColorRGB>
				</cc:DeviceColorValues>
			</cc:Object>)x";
static const char* txf_2 = R"x(
		</cc:ObjectCollection>
	</cc:Resources>
	<cc:CustomResources>
		<xrp:Prism xmlns:xrp="http://www.xrite.com/products/prism" release="2.0">
  <xrp:CustomAttributes
    NumberPatchColumns="29"
    PageHeight="279.40"
    PageWidth="215.90"
    PaperOrientation="Portrait"
    TitleString="%s"
    PatchSizeHeightValue="6.00"
    PatchSizeWidthValue="6.00"
    PrintMarginBottom="%4.2f"
    PrintMarginLeft="5.00"
    PrintMarginRight="5.00"
    PrintMarginTop="%4.2f"/>
</xrp:Prism>
	</cc:CustomResources>
</cc:CxF>
)x";

static const char* txf_2_Pro2 = R"x(
		</cc:ObjectCollection>
	</cc:Resources>
	<cc:CustomResources>
		<xrp:Prism xmlns:xrp="http://www.xrite.com/products/prism" release="2.0">
  <xrp:CustomAttributes
    NumberPatchColumns="29"
    PageHeight="215.90"
    PageWidth="279.40"
    PaperOrientation="Landscape"
    TitleString="%s"
    PatchSizeHeightValue="8.00"
    PatchSizeWidthValue="8.00"
    PrintMarginBottom="5.00"
    PrintMarginLeft="5.00"
    PrintMarginRight="5.00"
    PrintMarginTop="5.00"/>
</xrp:Prism>
	</cc:CustomResources>
</cc:CxF>
)x";
