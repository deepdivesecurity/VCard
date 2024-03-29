/**
 * Get all cards from upload folder
 * Parse cards to get relevant information
 * Populate CardView and FileLog panels with relevant information
 *   Both Tables must be populated correctly
 *   Dropdown in FileLog must be populated with all cards in uploads folder
 */
$(document).ready(function() {
    //Store and load status panel using localStorage (Persist on refresh)
    let currentStatus = localStorage.getItem("status");

    //If the status needs to be loaded, load the status panels
    if (currentStatus != null) {
        $("#status_text1").val(currentStatus);
        $("#status_text2").val(currentStatus);
    }
	
    //Variable to store file names retrieved from upload directory
    let listOfFileNames = [];

    //Write the copyright information for footer
    document.getElementById("copyright").innerHTML = formatCopyright();

    //Set uploadFileText to an empty string on reload
    $("#uploadFileText").val("");

    //Set upload button to disabled on reload
    $(".upload").attr("disabled", true);

    /**
     * Handles creating a back-to-top button while scrolling
     */
    $(window).scroll(function () {
        if ($(this).scrollTop() > 100) {
            $('#back-to-top').fadeIn();
        } else {
            $('#back-to-top').fadeOut();
        }
    });

    $('#back-to-top').click(function () {
        $('#back-to-top').tooltip();
        $('body,html').animate({
            scrollTop: 0
        }, 800);
        return false;
    });

    $('#back-to-top').tooltip();

    /**
     * Ajax to load all files found in upload directory into card view dropdown
     */
    $.ajax({
        type: 'get',
	dataType: 'json',
	url: '/uploadDirectory',
	success: function(data) {
	    listOfFileNames = data;

	    if (listOfFileNames.length === 0) {
                $("#file_log_table tbody").remove();
		//Set tables to "No Data"
                let newTable = "<tbody><tr>"
			+ "<td>No Files</td>"
			+ "<td>No Files</td>"
			+ "<td>No Files</td>"
			+ "</tr></tbody>";
		$("#file_log_table").append(newTable);
	    }
	    for (j = 0; j < listOfFileNames.length; j++) {
		let currentFile = listOfFileNames[j];
                /**
                 * Use listOfFileNames to create all of the necessary cards from
                 * upload folder.
                 * Populate File Log table using file name and returned JSON from 
                 * dynamic library function
                 */
                $("#file_log_table tbody").remove();
                $.ajax({
                    type: 'get',
	            dataType: 'json',
	            data: {file:currentFile},
	            url: '/fileLog',
	            success: function(data) {
			console.log(data);
                        $("#card_view_dropdown").append(new Option(currentFile, j));
			let newTable = "<tbody><tr>"
			+ "<td><a class=\"file_log_link\" href=\"uploads/" + currentFile + "\">" + currentFile + "</a></td>"
			+ "<td>" + data.indiname + "</td>"
			+ "<td>" + data.addiprops + "</td>"
			+ "</tr></tbody>";

	                $("#file_log_table").append(newTable);
	            },
	            fail: function(error) {
                        console.log(error);
	            }
                });
	    }
	},
	fail: function(error) {
            console.log(error);
	}
    });
});

/**
 * Handles file upload file selection
 */
$(document).on('click', '.browse', function() {
    let file = $(this).parent().parent().parent().find('.file');
    file.trigger('click');
});

/**
 * Handles file browsing and setting text for file to be uploaded
 */
$(document).on('change', '.file', function() {
    $(this).parent().find('.form-control').val($(this).val().replace(/C:\\fakepath\\/i, ''));
    let uploadFileName = $(this).parent().find('.form-control').val();
    let fileExtension = uploadFileName.substr(uploadFileName.indexOf('.'));

    if (fileExtension != '.vcf' && fileExtension != '.vcard') {
	let currentStatus = $("#status_text1").val();
	let newStatus = currentStatus + "Invalid file extension in upload form\n"
	$("#status_text1").val(newStatus);
	$("#status_text2").val(newStatus);
        $(this).parent().find('#uploadFileText').addClass('is-invalid');
    }
    else {
	$(this).parent().find('#uploadFileText').removeClass('is-invalid');
	$(this).parent().find('#uploadFileText').addClass('is-valid');
        $('.upload').prop('disabled', false);
    }
});

/**
 * Handles changing the status panel text during uploading of files
 */
$("#uploadForm").submit(function() {
    let currentStatus = $("#status_text1").val();
    let newStatus = currentStatus + "Uploaded " + $(this).find('.form-control').val() + "\n";
    $("#status_text1").val(newStatus);
    $("#status_text2").val(newStatus);
});

/**
 * Handles changing the status panel text during downloading of files
 */
$(document).on('click', '.file_log_link', function(e) {
    let text = $(e.target).text();
    let currentStatus = $("#status_text1").val();
    let newStatus = currentStatus +  "Downloaded " + text + "\n";
    $("#status_text1").val(newStatus);
    $("#status_text2").val(newStatus);
});

/**
 * Handles storing the current status panel content into local storage
 * for persistance on reload of window
 */
$(window).on('beforeunload', function() {
    //Create a new local storage item using the status panel content
    localStorage.setItem("status", $("#status_text1").val());
});

/**
 * Handles clearing the status textarea and local storage
 */
function clearText() {
    localStorage.clear();
    $("#status_text1").val("");
    $("#status_text2").val("");
}

/**
 * Handles activating/deactiving navigation tabs
 */
$(function() {
    $(".navbar-nav li").click(function() {
        // remove classes from all
        $(".navbar-nav li").removeClass("active");
        // add class to the one we clicked
        $(this).addClass("active");
    });
});

/**
 * Populates the cardView table on select.change event based on the file name
 * passed in
 */
$("#card_view_dropdown").change(function() {
    //Populate CardView Table with $(this).find('option:selected').text() file
    //Create card using file and return appropriate JSON for table manipulation
    //Add JSON content to table
    let selectedOption = $('#card_view_dropdown option:selected')
    if (selectedOption.text() != '- Select Card -') {
	$("#card_view_table tbody").remove();
	var currentFile = $(this).find('option:selected').text();
        $.ajax({
            type: 'get',
	    dataType: 'json',
	    data: {file:currentFile},
	    url: '/cardView',
	    success: function(data) {
                //Populate table using returned Object
		let newTable = "<tbody>"
		//Loop through every JSON Property returned
                $.each(data, function(index) {
		    let numberProp = index + 1;
		    newTable = newTable + "<tr><td>" + numberProp + "</td>"
		    + "<td>" + data[index].propname + "</td>"
	            + "<td>" + data[index].propvalues + "</td></tr>";
		});
		newTable = newTable + "</tbody>";

	        $("#card_view_table").append(newTable);
	    },
	    fail: function(error) {
                console.log(error);
	    }
        });
    }
    else {
        $("#card_view_table tbody").remove();
    }
});

/**
 * Retrieves the current date based on server settings
 */
function getDate() {
    let today = new Date();
    let year = today.getFullYear(); 
    let stringYear = year.toString();
    return stringYear;
}

/**
 * Formats the copyright information
 * @returns string
 */
function formatCopyright() {
    let date = getDate();
    let stringToReturn = "© " + date
    return stringToReturn;
}
